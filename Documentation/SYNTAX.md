# Syntax — API Reference

This document provides the complete API reference for the ST3215 C++ library, including all method signatures, parameters, return types, and usage examples.

## Namespace

All library types are in the `st3215` namespace:

```cpp
#include "st3215/st3215.h"
using namespace st3215;
```

## Constructor / Destructor

### `ST3215(const std::string& device)`

Opens the serial port and initializes communication with the servo bus.

| Parameter | Type | Description |
|-----------|------|-------------|
| `device` | `const std::string&` | Serial port path (e.g., `"/dev/ttyUSB0"`) |

**Throws:** `std::runtime_error` if the port cannot be opened.

```cpp
try {
    st3215::ST3215 servo("/dev/ttyUSB0");
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### `~ST3215()`

Closes the serial port and releases all resources.

---

## Servo Discovery

### `pingServo`

```cpp
bool pingServo(uint8_t sts_id);
```

Check if a servo is present and responding on the bus.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |

**Returns:** `true` if servo responds, `false` otherwise.

```cpp
if (servo.pingServo(1)) {
    std::cout << "Servo 1 is online" << std::endl;
}
```

### `listServos`

```cpp
std::vector<uint8_t> listServos();
```

Scan the entire bus (IDs 0-253) and return all responding servo IDs.

**Returns:** `std::vector<uint8_t>` containing IDs of all found servos.

```cpp
auto servos = servo.listServos();
for (auto id : servos) {
    std::cout << "Found servo: " << static_cast<int>(id) << std::endl;
}
```

---

## Read Operations

### `readPosition`

```cpp
std::optional<uint16_t> readPosition(uint8_t sts_id);
```

Read the current angular position of the servo.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |

**Returns:** Position (0-4095), or `std::nullopt` on error.

```cpp
auto pos = servo.readPosition(1);
if (pos.has_value()) {
    std::cout << "Position: " << pos.value() << std::endl;
}
```

### `readSpeed`

```cpp
std::tuple<int16_t, int, uint8_t> readSpeed(uint8_t sts_id);
```

Read the current speed of the servo. Returns signed speed (negative = counterclockwise).

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |

**Returns:** Tuple of `(speed, comm_result, error)`.

```cpp
auto [speed, comm, error] = servo.readSpeed(1);
if (comm == st3215::COMM_SUCCESS) {
    std::cout << "Speed: " << speed << " step/s" << std::endl;
}
```

### `readVoltage`

```cpp
std::optional<double> readVoltage(uint8_t sts_id);
```

Read the supply voltage of the servo in volts.

**Returns:** Voltage in V, or `std::nullopt` on error.

```cpp
auto volt = servo.readVoltage(1);
if (volt) std::cout << "Voltage: " << *volt << " V" << std::endl;
```

### `readCurrent`

```cpp
std::optional<double> readCurrent(uint8_t sts_id);
```

Read the motor current in milliamps.

**Returns:** Current in mA, or `std::nullopt` on error.

### `readTemperature`

```cpp
std::optional<int> readTemperature(uint8_t sts_id);
```

Read the internal temperature of the servo in degrees Celsius.

**Returns:** Temperature in °C, or `std::nullopt` on error.

### `readLoad`

```cpp
std::optional<double> readLoad(uint8_t sts_id);
```

Read the motor load as a percentage (duty cycle of current control output).

**Returns:** Load in %, or `std::nullopt` on error.

### `readAcceleration`

```cpp
std::optional<uint8_t> readAcceleration(uint8_t sts_id);
```

Read the current acceleration setting.

**Returns:** Acceleration value (0-254), or `std::nullopt` on error.

### `readMode`

```cpp
std::optional<uint8_t> readMode(uint8_t sts_id);
```

Read the current operating mode.

**Returns:** Mode (0=Position, 1=Speed, 2=PWM, 3=Step), or `std::nullopt` on error.

### `readCorrection`

```cpp
std::optional<int16_t> readCorrection(uint8_t sts_id);
```

Read the current position correction value.

**Returns:** Signed correction in steps, or `std::nullopt` on error.

### `isMoving`

```cpp
std::optional<bool> isMoving(uint8_t sts_id);
```

Check if the servo is currently in motion.

**Returns:** `true` if moving, `false` if stopped, `std::nullopt` on error.

### `readStatus`

```cpp
std::optional<std::map<std::string, bool>> readStatus(uint8_t sts_id);
```

Read the hardware status of all servo sensors.

**Returns:** Map with keys `"Voltage"`, `"Sensor"`, `"Temperature"`, `"Current"`, `"Angle"`, `"Overload"`. Values are `true` for OK, `false` for error. Returns `std::nullopt` on communication error.

```cpp
auto status = servo.readStatus(1);
if (status) {
    for (const auto& [name, ok] : *status) {
        std::cout << name << ": " << (ok ? "OK" : "ERROR") << std::endl;
    }
}
```

---

## Write Operations

### `moveTo`

```cpp
bool moveTo(uint8_t sts_id, uint16_t position, uint16_t speed = 2400,
            uint8_t acc = 50, bool wait = false);
```

Move the servo to a target position with configurable speed and acceleration.

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| `sts_id` | `uint8_t` | 0-253 | — | Servo ID |
| `position` | `uint16_t` | 0-4095 | — | Target position |
| `speed` | `uint16_t` | 0-3400 | 2400 | Speed (step/s) |
| `acc` | `uint8_t` | 0-254 | 50 | Acceleration (×100 step/s²) |
| `wait` | `bool` | — | `false` | Block until position is reached |

**Returns:** `true` on success, `false` on error.

```cpp
// Simple move
servo.moveTo(1, 2048);

// Move with custom speed, wait for completion
servo.moveTo(1, 3000, 1500, 100, true);
```

### `writePosition`

```cpp
bool writePosition(uint8_t sts_id, uint16_t position);
```

Low-level position write. Does not set mode, speed, or acceleration.

**Returns:** `true` on success, `false` on error.

### `setSpeed`

```cpp
bool setSpeed(uint8_t sts_id, uint16_t speed);
```

Set the movement speed.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |
| `speed` | `uint16_t` | 0-3400 | Speed in step/s |

**Returns:** `true` on success.

### `setAcceleration`

```cpp
bool setAcceleration(uint8_t sts_id, uint8_t acc);
```

Set the acceleration.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |
| `acc` | `uint8_t` | 0-254 | Acceleration (×100 step/s²) |

**Returns:** `true` on success.

### `rotate`

```cpp
bool rotate(uint8_t sts_id, int16_t speed);
```

Start continuous rotation. Automatically sets the servo to Speed mode (mode 1).

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |
| `speed` | `int16_t` | ±3400 | Speed (negative = counterclockwise) |

**Returns:** `true` on success.

```cpp
servo.rotate(1, 500);    // Clockwise at 500 step/s
servo.rotate(1, -500);   // Counterclockwise at 500 step/s
```

### `stopServo`

```cpp
bool stopServo(uint8_t sts_id);
```

Stop the servo by disabling torque (sets torque enable to 0).

**Returns:** `true` on success.

### `startServo`

```cpp
bool startServo(uint8_t sts_id);
```

Start the servo by enabling torque (sets torque enable to 1).

**Returns:** `true` on success.

### `setMode`

```cpp
bool setMode(uint8_t sts_id, uint8_t mode);
```

Set the operating mode.

| Mode | Value | Description |
|------|-------|-------------|
| Position | 0 | Move to target position |
| Speed | 1 | Continuous rotation |
| PWM | 2 | Direct PWM control |
| Step | 3 | Step servo mode |

**Returns:** `true` on success.

### `correctPosition`

```cpp
bool correctPosition(uint8_t sts_id, int16_t correction);
```

Set a position correction offset.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |
| `correction` | `int16_t` | ±2047 | Correction in steps (positive or negative) |

**Returns:** `true` on success.

---

## Configuration Operations

### `changeId`

```cpp
std::string changeId(uint8_t sts_id, uint8_t new_id);
```

Change the servo's ID. Automatically handles EEPROM unlock/lock.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Current servo ID |
| `new_id` | `uint8_t` | 0-253 | New servo ID |

**Returns:** Empty string `""` on success, error message on failure.

```cpp
auto result = servo.changeId(1, 5);
if (result.empty()) {
    std::cout << "ID changed to 5" << std::endl;
} else {
    std::cerr << result << std::endl;
}
```

### `changeBaudrate`

```cpp
std::string changeBaudrate(uint8_t sts_id, uint8_t new_baudrate);
```

Change the servo's baudrate. Automatically handles EEPROM unlock/lock.

| Parameter | Type | Range | Description |
|-----------|------|-------|-------------|
| `sts_id` | `uint8_t` | 0-253 | Servo ID |
| `new_baudrate` | `uint8_t` | 0-7 | Baudrate code (see Communication doc) |

**Returns:** Empty string on success, error message on failure.

### `lockEprom`

```cpp
int lockEprom(uint8_t sts_id);
```

Lock the EEPROM to prevent writes. Returns communication result code.

### `unlockEprom`

```cpp
int unlockEprom(uint8_t sts_id);
```

Unlock the EEPROM to allow writes. Returns communication result code.

---

## Calibration Operations

### `defineMiddle`

```cpp
bool defineMiddle(uint8_t sts_id);
```

Define the current physical position as the middle (2048) by setting torque to 128.

**Returns:** `true` on success.

### `tareServo`

```cpp
std::tuple<std::optional<uint16_t>, std::optional<uint16_t>> tareServo(uint8_t sts_id);
```

Calibrate a servo by finding its physical min/max limits. The servo will rotate in both directions until blocked, then calculate and apply a position correction to center the range.

**Warning:** Only use for servos with physical stops. Do not use for free-rotation servos.

**Returns:** Tuple of `(min_position, max_position)`. Both are `std::nullopt` on error.

```cpp
auto [min_pos, max_pos] = servo.tareServo(1);
if (min_pos && max_pos) {
    std::cout << "Range: " << *min_pos << " to " << *max_pos << std::endl;
}
```

---

## GroupSyncWrite

### Constructor

```cpp
GroupSyncWrite(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length);
```

The ST3215 class creates one automatically:
```cpp
servo.groupSyncWrite  // Pre-initialized with STS_ACC, data_length=7
```

### Methods

```cpp
bool addParam(uint8_t sts_id, const std::vector<uint8_t>& data);
void removeParam(uint8_t sts_id);
bool changeParam(uint8_t sts_id, const std::vector<uint8_t>& data);
void clearParam();
int txPacket();
```

### Example: Move Two Servos Simultaneously

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

// Data format: [ACC, POS_L, POS_H, TIME_L, TIME_H, SPEED_L, SPEED_H]
std::vector<uint8_t> data1 = {50, 0x00, 0x08, 0x00, 0x00, 0x60, 0x09};  // pos=2048, speed=2400
std::vector<uint8_t> data2 = {50, 0x00, 0x0C, 0x00, 0x00, 0x60, 0x09};  // pos=3072, speed=2400

servo.groupSyncWrite->addParam(1, data1);
servo.groupSyncWrite->addParam(2, data2);
servo.groupSyncWrite->txPacket();
servo.groupSyncWrite->clearParam();
```

---

## GroupSyncRead

### Constructor

```cpp
GroupSyncRead(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length);
```

### Methods

```cpp
bool addParam(uint8_t sts_id);
void removeParam(uint8_t sts_id);
void clearParam();
int txRxPacket();
std::tuple<bool, uint8_t> isAvailable(uint8_t sts_id, uint8_t address, uint8_t data_length);
uint32_t getData(uint8_t sts_id, uint8_t address, uint8_t data_length);
```

### Example: Read Positions from Two Servos

```cpp
st3215::GroupSyncRead syncRead(&servo, st3215::STS_PRESENT_POSITION_L, 2);
syncRead.addParam(1);
syncRead.addParam(2);

int result = syncRead.txRxPacket();
if (result == st3215::COMM_SUCCESS) {
    auto [avail1, err1] = syncRead.isAvailable(1, st3215::STS_PRESENT_POSITION_L, 2);
    auto [avail2, err2] = syncRead.isAvailable(2, st3215::STS_PRESENT_POSITION_L, 2);

    if (avail1) {
        uint32_t pos1 = syncRead.getData(1, st3215::STS_PRESENT_POSITION_L, 2);
        std::cout << "Servo 1 position: " << pos1 << std::endl;
    }
    if (avail2) {
        uint32_t pos2 = syncRead.getData(2, st3215::STS_PRESENT_POSITION_L, 2);
        std::cout << "Servo 2 position: " << pos2 << std::endl;
    }
}
```

---

## Protocol Layer Methods

These lower-level methods are available through the `ProtocolPacketHandler` base class:

### Read Methods

```cpp
std::tuple<std::vector<uint8_t>, int, uint8_t> readTxRx(uint8_t sts_id, uint8_t address, uint8_t length);
std::tuple<uint8_t, int, uint8_t> read1ByteTxRx(uint8_t sts_id, uint8_t address);
std::tuple<uint16_t, int, uint8_t> read2ByteTxRx(uint8_t sts_id, uint8_t address);
std::tuple<uint32_t, int, uint8_t> read4ByteTxRx(uint8_t sts_id, uint8_t address);
```

### Write Methods

```cpp
int writeTxOnly(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);
std::tuple<int, uint8_t> writeTxRx(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);
int write1ByteTxOnly(uint8_t sts_id, uint8_t address, uint8_t data);
int write2ByteTxOnly(uint8_t sts_id, uint8_t address, uint16_t data);
int write4ByteTxOnly(uint8_t sts_id, uint8_t address, uint32_t data);
```

### Utility Methods

```cpp
uint16_t makeWord(uint8_t a, uint8_t b) const;
uint32_t makeDWord(uint16_t a, uint16_t b) const;
uint8_t lobyte(uint16_t w) const;
uint8_t hibyte(uint16_t w) const;
uint16_t loword(uint32_t l) const;
uint16_t hiword(uint32_t h) const;
int16_t toHost(uint16_t a, uint8_t b) const;
int16_t toScs(int16_t a, uint8_t b) const;
void setEnd(uint8_t end);
uint8_t getEnd() const;
```
