# Architecture

This document describes the system architecture of the ST3215 C++ library, including its layered design, class hierarchy, component responsibilities, and data flow.

## Layered Architecture

The library follows a three-layer architecture, mirroring the original Python library:

```
┌──────────────────────────────────────────────────────┐
│                   Application Layer                  │
│                                                      │
│   ST3215 class  ·  GroupSyncWrite  ·  GroupSyncRead   │
│   (High-level servo control API)                     │
├──────────────────────────────────────────────────────┤
│                   Protocol Layer                     │
│                                                      │
│              ProtocolPacketHandler                    │
│   (Packet construction, parsing, checksums)           │
├──────────────────────────────────────────────────────┤
│                  Transport Layer                     │
│                                                      │
│                   PortHandler                        │
│   (Serial port I/O, timing, baudrate)                │
└──────────────────────────────────────────────────────┘
```

### Layer Responsibilities

**Application Layer** (`ST3215`, `GroupSyncWrite`, `GroupSyncRead`)
- Provides user-facing API methods like `moveTo()`, `readPosition()`, `pingServo()`
- Manages servo-level logic (mode switching, calibration, motion wait)
- Orchestrates multi-servo synchronized operations
- Translates between human-readable values and raw register data

**Protocol Layer** (`ProtocolPacketHandler`)
- Constructs STS protocol packets with headers, IDs, instructions, and checksums
- Parses response packets and validates checksums
- Handles byte-order conversion (endianness)
- Provides typed read/write operations (1-byte, 2-byte, 4-byte)

**Transport Layer** (`PortHandler`)
- Opens, configures, and closes the serial port
- Manages baudrate, 8N1 framing, and raw byte I/O
- Implements packet timeouts and timing
- Provides buffer management (clear, flush)

## Class Hierarchy

```
                  ProtocolPacketHandler
                         ▲
                         │ (inherits)
                         │
                       ST3215
                      /      \
                     /        \  (owns via unique_ptr)
                    /          \
          PortHandler    GroupSyncWrite
```

### Inheritance

`ST3215` inherits from `ProtocolPacketHandler`. This allows `ST3215` to directly call protocol-level methods like `read2ByteTxRx()` and `writeTxRx()` without indirection.

### Composition

- `ST3215` owns a `PortHandler` via `std::unique_ptr<PortHandler>`. The port handler is created in the constructor and destroyed automatically.
- `ST3215` owns a `GroupSyncWrite` via `std::unique_ptr<GroupSyncWrite>`, initialized with start address `STS_ACC` and data length 7 (matching the Python library).
- `GroupSyncRead` is a standalone class that can be instantiated by user code when needed.

## Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                         User Code                           │
│                                                             │
│   #include "st3215/st3215.h"                                │
│   ST3215 servo("/dev/ttyUSB0");                             │
│   servo.moveTo(1, 2048);                                    │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                      ST3215 Class                           │
│                                                             │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────────┐  │
│  │  Discovery   │  │  Telemetry   │  │    Motion Control  │  │
│  │  pingServo() │  │  readPos()   │  │    moveTo()        │  │
│  │  listServos()│  │  readVolt()  │  │    rotate()        │  │
│  │              │  │  readTemp()  │  │    setSpeed()      │  │
│  └──────┬───────┘  └──────┬───────┘  └─────────┬──────────┘  │
│         │                 │                    │              │
│         └─────────────────┼────────────────────┘              │
│                           ▼                                  │
│  ┌──────────────────────────────────────────────────────┐    │
│  │              ProtocolPacketHandler (base)             │    │
│  │                                                      │    │
│  │  read1ByteTxRx()    write1ByteTxOnly()               │    │
│  │  read2ByteTxRx()    write2ByteTxOnly()               │    │
│  │  ping()             writeTxRx()                      │    │
│  │  syncReadTx()       syncWriteTxOnly()                │    │
│  │  txPacket()         rxPacket()                       │    │
│  └──────────────────────┬───────────────────────────────┘    │
│                         │                                    │
│                         ▼                                    │
│  ┌──────────────────────────────────────────────────────┐    │
│  │                    PortHandler                        │    │
│  │                                                      │    │
│  │  openPort()        readPort()       setBaudRate()     │    │
│  │  closePort()       writePort()      setPacketTimeout()│    │
│  │  clearPort()       getBytesAvailable()                │    │
│  └──────────────────────┬───────────────────────────────┘    │
│                         │                                    │
└─────────────────────────┼────────────────────────────────────┘
                          │
                          ▼
                   ┌──────────────┐
                   │  Serial Port │
                   │ /dev/ttyUSB0 │
                   └──────────────┘
```

## Data Flow

### Write Operation (e.g., `moveTo`)

```
User calls servo.moveTo(1, 2048)
  │
  ├─► ST3215::setMode(1, 0)           [set position mode]
  │     └─► writeTxRx(1, STS_MODE, ...)
  │           └─► txRxPacket(txpacket)
  │                 ├─► txPacket()     [build + send packet]
  │                 │     └─► PortHandler::writePort()
  │                 └─► rxPacket()     [receive + validate response]
  │                       └─► PortHandler::readPort()
  │
  ├─► ST3215::setAcceleration(1, 50)   [set acceleration]
  ├─► ST3215::setSpeed(1, 2400)        [set speed]
  └─► ST3215::writePosition(1, 2048)   [set target position]
```

### Read Operation (e.g., `readPosition`)

```
User calls servo.readPosition(1)
  │
  └─► ProtocolPacketHandler::read2ByteTxRx(1, STS_PRESENT_POSITION_L)
        └─► readTxRx(1, 56, 2)
              └─► txRxPacket(txpacket)
                    ├─► txPacket()          [send READ instruction]
                    │     └─► writePort()
                    └─► rxPacket()          [receive 2 data bytes]
                          └─► readPort()
                                │
                                ▼
                        makeWord(lo, hi)    [combine into uint16_t]
                                │
                                ▼
                        return std::optional<uint16_t>(position)
```

## Thread Safety Model

The library uses a `std::mutex` in the `ST3215` class. The `PortHandler` also has an `is_using_` flag to prevent concurrent bus access at the transport level. This two-level protection ensures:

1. Only one thread can execute a high-level operation at a time
2. The serial bus is never accessed concurrently
3. Packet sequences (TX then RX) are atomic

## File Organization

```
cpp_ST3215/
├── include/st3215/
│   ├── st3215.h                    # Main API class
│   ├── protocol_packet_handler.h   # Protocol layer
│   ├── port_handler.h              # Transport layer
│   ├── group_sync_write.h          # Sync write operations
│   ├── group_sync_read.h           # Sync read operations
│   └── values.h                    # Constants and register map
├── src/
│   ├── st3215.cpp
│   ├── protocol_packet_handler.cpp
│   ├── port_handler.cpp
│   ├── group_sync_write.cpp
│   └── group_sync_read.cpp
├── examples/
│   ├── ping_servo.cpp
│   ├── list_servos.cpp
│   ├── move_servo.cpp
│   └── read_telemetry.cpp
├── Documentation/                  # This documentation
└── CMakeLists.txt                  # Build system
```

## Comparison with Python Architecture

| Aspect | Python | C++ |
|--------|--------|-----|
| Inheritance | `ST3215` inherits `protocol_packet_handler` | Same: `ST3215` inherits `ProtocolPacketHandler` |
| Port ownership | Instance variable (`self.portHandler`) | `std::unique_ptr<PortHandler>` |
| Sync write | `GroupSyncWrite` with protocol handler ref | Same, via pointer |
| Sync read | `GroupSyncRead` with protocol handler ref | Same, via pointer |
| Thread safety | `threading.Lock()` + GIL | `std::mutex` (true parallelism) |
| Error handling | Returns `None` on error | Returns `std::nullopt` via `std::optional` |
| Byte order | `self.sts_end` attribute | `sts_end_` member variable |
| Serial I/O | `pyserial` library | POSIX `termios` API |
