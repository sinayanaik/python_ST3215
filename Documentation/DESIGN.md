# Design

This document covers the design patterns, decisions, trade-offs, and rationale behind the ST3215 C++ library.

## Design Goals

1. **Functional parity** with the Python ST3215 library
2. **Type safety** using modern C++17 features
3. **Memory safety** via RAII and smart pointers
4. **Thread safety** for concurrent servo access
5. **Minimal dependencies** (standard library + POSIX only)
6. **Easy integration** via CMake package config

## Design Decisions

### 1. Inheritance vs Composition for Protocol Handler

**Decision:** `ST3215` inherits from `ProtocolPacketHandler`

**Rationale:**
- Matches the Python library design (`class ST3215(protocol_packet_handler)`)
- Allows direct access to protocol methods without wrapper functions
- The "is-a" relationship is appropriate: ST3215 "is a" protocol handler specialized for servo control

**Trade-off:** Tighter coupling, but simplifies the API and matches the reference implementation.

### 2. std::optional for Error Handling

**Decision:** Use `std::optional<T>` for read operations that can fail

```cpp
std::optional<uint16_t> readPosition(uint8_t sts_id);
```

**Rationale:**
- The Python library returns `None` for failed reads — `std::optional` is the direct C++ equivalent
- Distinguishes between "no value" and "value of zero" (which is a valid position)
- Forces callers to check for errors before using the value
- No exception overhead for expected failures (communication errors are common)

**Alternatives considered:**
- **Exceptions:** Rejected because comm failures are expected, not exceptional
- **Error codes + out parameters:** Rejected because it's verbose and error-prone
- **std::expected (C++23):** Not available in C++17

### 3. Smart Pointers for Ownership

**Decision:** `std::unique_ptr<PortHandler>` for port ownership

```cpp
class ST3215 {
    std::unique_ptr<PortHandler> port_handler_;
    std::unique_ptr<GroupSyncWrite> groupSyncWrite;
};
```

**Rationale:**
- Automatic cleanup when ST3215 is destroyed (RAII)
- Clear single-ownership semantics
- No possibility of memory leaks
- Exception-safe construction and destruction

### 4. Mutex for Thread Safety

**Decision:** Single `std::mutex` in the ST3215 class, plus `is_using_` flag in PortHandler

**Rationale:**
- Serial communication is inherently sequential (half-duplex bus)
- A single mutex is simple and correct
- The `is_using_` flag at the port level provides an additional safety layer
- Matches the Python approach (`threading.Lock()`) but with true thread safety (no GIL)

### 5. Constants as constexpr

**Decision:** All protocol constants defined as `constexpr` in `values.h`

```cpp
constexpr uint8_t STS_PRESENT_POSITION_L = 56;
constexpr uint16_t MAX_POSITION = 4095;
```

**Rationale:**
- Zero runtime cost (compile-time constants)
- Type-safe (unlike `#define` macros)
- Scoped to the `st3215` namespace (no global pollution)
- Matches the Python `values.py` constants exactly

### 6. Return Types for Write Operations

**Decision:** Write operations return `bool` (true on success, false on failure)

```cpp
bool setAcceleration(uint8_t sts_id, uint8_t acc);
bool moveTo(uint8_t sts_id, uint16_t position, ...);
```

**Rationale:**
- Simple and clear: did the operation succeed?
- Consistent across all write methods
- The Python library inconsistently returns `True`, `None`, or raw tuples — the C++ version normalizes this

**Note:** The Python `StartServo()` and `SetMode()` return the raw `(comm, error)` tuple, while `StopServo()` and `SetAcceleration()` return `True`/`None`. The C++ version makes all write operations return `bool` for consistency.

### 7. String Return for Configuration Changes

**Decision:** `changeId()` and `changeBaudrate()` return `std::string`

```cpp
std::string changeId(uint8_t sts_id, uint8_t new_id);
// Returns "" on success, error message on failure
```

**Rationale:**
- Matches the Python API where these return `None` on success or an error string
- In C++, empty string = success, non-empty = error with description
- Provides human-readable error messages

### 8. GroupSyncWrite Initialization

**Decision:** ST3215 constructor creates a `GroupSyncWrite` with start address `STS_ACC` and data length 7

```cpp
groupSyncWrite = std::make_unique<GroupSyncWrite>(this, STS_ACC, 7);
```

**Rationale:**
- Exact match of Python: `self.groupSyncWrite = GroupSyncWrite(self, STS_ACC, 7)`
- Ready to use for synchronized multi-servo writes
- Data length 7 covers: ACC(1) + GOAL_POS(2) + GOAL_TIME(2) + GOAL_SPEED(2)

## Design Patterns

### RAII (Resource Acquisition Is Initialization)

All resources are acquired in constructors and released in destructors:

```cpp
ST3215::ST3215(const std::string& device) {
    port_handler_ = std::make_unique<PortHandler>(device);
    port_handler_->openPort();  // acquire
}

ST3215::~ST3215() {
    port_handler_->closePort();  // release
}
```

### Template Method Pattern

`ProtocolPacketHandler` provides the general packet TX/RX mechanism. Derived classes (`ST3215`) use these building blocks to implement servo-specific operations.

```
readPosition() → read2ByteTxRx() → readTxRx() → txRxPacket() → txPacket() + rxPacket()
```

### Builder Pattern (Packet Construction)

Packets are built incrementally:

```cpp
txpacket[PKT_HEADER_0] = 0xFF;
txpacket[PKT_HEADER_1] = 0xFF;
txpacket[PKT_ID] = sts_id;
txpacket[PKT_LENGTH] = length + 3;
txpacket[PKT_INSTRUCTION] = INST_WRITE;
txpacket[PKT_PARAMETER0] = address;
// ... add data bytes ...
// ... compute and add checksum ...
```

### Facade Pattern

`ST3215` acts as a facade over the protocol and transport layers, providing a simple API:

```cpp
// User code — simple
servo.moveTo(1, 2048);

// Behind the scenes — complex
// setMode → setAcceleration → setSpeed → readPosition → writePosition → wait
```

## Error Handling Strategy

### Three Levels of Error Handling

1. **Exceptions** — For fatal errors (port cannot be opened)
   ```cpp
   throw std::runtime_error("Could not open port: " + device);
   ```

2. **std::optional** — For read operations that may fail
   ```cpp
   return std::nullopt;  // Communication error
   ```

3. **Boolean return** — For write operations
   ```cpp
   return (comm == COMM_SUCCESS && error == 0);
   ```

### Error Propagation

Errors propagate upward through the layers:

```
PortHandler::writePort() → returns 0 bytes written
  ↓
txPacket() → returns COMM_TX_FAIL
  ↓
txRxPacket() → returns (rxpacket, COMM_TX_FAIL, 0)
  ↓
read2ByteTxRx() → returns (0, COMM_TX_FAIL, 0)
  ↓
readPosition() → returns std::nullopt
  ↓
User checks: if (pos.has_value()) { ... }
```

## Endianness Design

The ST3215 servo uses little-endian byte order. The library handles this via the `sts_end_` member:

```cpp
// sts_end_ = 0: little-endian (default, used by ST3215)
uint16_t makeWord(uint8_t lo, uint8_t hi) {
    return (lo & 0xFF) | ((hi & 0xFF) << 8);  // lo byte first
}

// sts_end_ = 1: big-endian (used by SCS servos)
uint16_t makeWord(uint8_t lo, uint8_t hi) {
    return (hi & 0xFF) | ((lo & 0xFF) << 8);  // hi byte first
}
```

This design supports both ST (little-endian) and SCS (big-endian) servo families.

## Timing Design

### Packet Timeout Calculation

```
timeout = (tx_time_per_byte × packet_length) + (tx_time_per_byte × 3) + LATENCY_TIMER
```

Where:
- `tx_time_per_byte = (1000 / baudrate) × 10` milliseconds
- `LATENCY_TIMER = 50` ms (accounts for USB-serial adapter latency)
- Factor of 3 provides margin for processing delay

### Motion Wait Calculation

When `moveTo()` is called with `wait=true`, the library calculates the expected motion duration using kinematics (see [Math documentation](MATH.md)).

## Python Parity Mapping

| Python Pattern | C++ Equivalent |
|---------------|----------------|
| `return None` | `return std::nullopt` |
| `return True` | `return true` |
| `threading.Lock()` | `std::mutex lock_` |
| `self.portHandler` | `std::unique_ptr<PortHandler>` |
| `GroupSyncWrite(self, STS_ACC, 7)` | `std::make_unique<GroupSyncWrite>(this, STS_ACC, 7)` |
| `time.sleep(0.5)` | `std::this_thread::sleep_for(500ms)` |
| `abs(speed)` | `std::abs(speed)` |
| `math.sqrt(x)` | `std::sqrt(x)` |
| `isinstance(new_id, int) and 0 <= new_id <= 253` | `new_id > 253` (type enforced at compile time) |
