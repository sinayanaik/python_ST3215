# Code

This document describes the code structure, naming conventions, file layout, coding standards, and contribution guidelines for the ST3215 C++ library.

## File Layout

```
cpp_ST3215/
├── CMakeLists.txt                          # Root build configuration
├── README.md                               # Project overview
├── LICENSE                                 # MIT license
├── QUICKSTART.md                           # Getting started guide
├── CONTRIBUTING.md                         # Contribution guidelines
├── TECHNICAL_COMPARISON.md                 # Python vs C++ comparison
├── IMPLEMENTATION_NOTES.md                 # Implementation details
├── .gitignore                              # Git ignore rules
│
├── include/st3215/                         # Public header files
│   ├── st3215.h                            # Main API class
│   ├── protocol_packet_handler.h           # Protocol layer
│   ├── port_handler.h                      # Transport layer
│   ├── group_sync_write.h                  # Sync write
│   ├── group_sync_read.h                   # Sync read
│   └── values.h                            # Constants
│
├── src/                                    # Implementation files
│   ├── st3215.cpp                          # Main API implementation
│   ├── protocol_packet_handler.cpp         # Protocol implementation
│   ├── port_handler.cpp                    # Transport implementation
│   ├── group_sync_write.cpp                # Sync write implementation
│   └── group_sync_read.cpp                 # Sync read implementation
│
├── examples/                               # Example programs
│   ├── CMakeLists.txt                      # Examples build config
│   ├── ping_servo.cpp                      # Ping a servo
│   ├── list_servos.cpp                     # Scan for servos
│   ├── move_servo.cpp                      # Move a servo
│   └── read_telemetry.cpp                  # Read sensor data
│
├── cmake/                                  # CMake config templates
│   └── ST3215Config.cmake.in               # Package config
│
└── Documentation/                          # Detailed documentation
    ├── README.md                           # Doc index
    ├── ARCHITECTURE.md                     # System architecture
    ├── ENGINEERING.md                      # Build and tooling
    ├── DESIGN.md                           # Design decisions
    ├── CODE.md                             # This file
    ├── MATH.md                             # Motion math
    ├── COMMUNICATION.md                    # Protocol details
    ├── SYNTAX.md                           # API reference
    └── INSTALLATION.md                     # Install guide
```

## Naming Conventions

### Classes

PascalCase for all class names:

```cpp
class ST3215 { ... };
class PortHandler { ... };
class ProtocolPacketHandler { ... };
class GroupSyncWrite { ... };
class GroupSyncRead { ... };
```

### Methods

camelCase for all method names:

```cpp
bool pingServo(uint8_t sts_id);
std::optional<uint16_t> readPosition(uint8_t sts_id);
bool moveTo(uint8_t sts_id, uint16_t position, ...);
int txPacket(std::vector<uint8_t>& txpacket);
```

### Member Variables

snake_case with trailing underscore:

```cpp
std::unique_ptr<PortHandler> port_handler_;
std::mutex lock_;
uint8_t sts_end_;
bool is_open_;
int serial_fd_;
```

### Constants

UPPER_SNAKE_CASE:

```cpp
constexpr uint32_t DEFAULT_BAUDRATE = 1000000;
constexpr uint16_t MAX_POSITION = 4095;
constexpr uint8_t INST_READ = 2;
constexpr uint8_t STS_PRESENT_POSITION_L = 56;
```

### Parameters

snake_case:

```cpp
bool moveTo(uint8_t sts_id, uint16_t position, uint16_t speed, uint8_t acc);
```

### Namespaces

All library code is in the `st3215` namespace:

```cpp
namespace st3215 {
    class ST3215 { ... };
}
```

## Code Style

### Indentation

4 spaces, no tabs.

### Braces

Opening brace on the same line:

```cpp
if (condition) {
    // code
} else {
    // code
}

bool ST3215::pingServo(uint8_t sts_id) {
    // code
}
```

### Line Length

120 characters maximum.

### Include Order

1. Project headers (quoted)
2. Standard library headers (angle brackets)
3. System headers (angle brackets)

```cpp
#include "st3215/st3215.h"
#include <iostream>
#include <vector>
#include <thread>
#include <fcntl.h>
#include <termios.h>
```

### Documentation Style

Doxygen-style comments for all public methods:

```cpp
/**
 * @brief Move servo to target position
 * @param sts_id Servo ID
 * @param position Target position (0-4095)
 * @param speed Movement speed (default: 2400 step/s)
 * @param acc Acceleration (default: 50, unit: 100 step/s²)
 * @param wait Wait for movement to complete (default: false)
 * @return true on success, false on error
 */
bool moveTo(uint8_t sts_id, uint16_t position, uint16_t speed = 2400,
            uint8_t acc = 50, bool wait = false);
```

## Python to C++ Mapping

### Method Name Mapping

| Python Method | C++ Method | Notes |
|--------------|------------|-------|
| `PingServo()` | `pingServo()` | camelCase |
| `ListServos()` | `listServos()` | camelCase |
| `ReadLoad()` | `readLoad()` | camelCase |
| `ReadVoltage()` | `readVoltage()` | camelCase |
| `ReadCurrent()` | `readCurrent()` | camelCase |
| `ReadTemperature()` | `readTemperature()` | camelCase |
| `ReadAccelaration()` | `readAcceleration()` | Fixed typo in name |
| `ReadMode()` | `readMode()` | camelCase |
| `ReadCorrection()` | `readCorrection()` | camelCase |
| `IsMoving()` | `isMoving()` | camelCase |
| `ReadPosition()` | `readPosition()` | camelCase |
| `ReadSpeed()` | `readSpeed()` | camelCase |
| `ReadStatus()` | `readStatus()` | camelCase |
| `SetAcceleration()` | `setAcceleration()` | camelCase |
| `SetSpeed()` | `setSpeed()` | camelCase |
| `StopServo()` | `stopServo()` | camelCase |
| `StartServo()` | `startServo()` | camelCase |
| `SetMode()` | `setMode()` | camelCase |
| `CorrectPosition()` | `correctPosition()` | camelCase |
| `Rotate()` | `rotate()` | camelCase |
| `MoveTo()` | `moveTo()` | camelCase |
| `WritePosition()` | `writePosition()` | camelCase |
| `DefineMiddle()` | `defineMiddle()` | camelCase |
| `TareServo()` | `tareServo()` | camelCase |
| `LockEprom()` | `lockEprom()` | camelCase |
| `UnLockEprom()` | `unlockEprom()` | camelCase |
| `ChangeId()` | `changeId()` | camelCase |
| `ChangeBaudrate()` | `changeBaudrate()` | camelCase |
| `getBlockPosition()` | `getBlockPosition()` | Same (private) |

### Protocol Method Mapping

| Python Method | C++ Method |
|--------------|------------|
| `ping()` | `ping()` |
| `action()` | `action()` |
| `readTx()` | `readTx()` |
| `readRx()` | `readRx()` |
| `readTxRx()` | `readTxRx()` |
| `read1ByteTx()` | `read1ByteTx()` |
| `read1ByteRx()` | `read1ByteRx()` |
| `read1ByteTxRx()` | `read1ByteTxRx()` |
| `read2ByteTx()` | `read2ByteTx()` |
| `read2ByteRx()` | `read2ByteRx()` |
| `read2ByteTxRx()` | `read2ByteTxRx()` |
| `read4ByteTx()` | `read4ByteTx()` |
| `read4ByteRx()` | `read4ByteRx()` |
| `read4ByteTxRx()` | `read4ByteTxRx()` |
| `writeTxOnly()` | `writeTxOnly()` |
| `writeTxRx()` | `writeTxRx()` |
| `write1ByteTxOnly()` | `write1ByteTxOnly()` |
| `write1ByteTxRx()` | `write1ByteTxRx()` |
| `write2ByteTxOnly()` | `write2ByteTxOnly()` |
| `write2ByteTxRx()` | `write2ByteTxRx()` |
| `write4ByteTxOnly()` | `write4ByteTxOnly()` |
| `write4ByteTxRx()` | `write4ByteTxRx()` |
| `regWriteTxOnly()` | `regWriteTxOnly()` |
| `regWriteTxRx()` | `regWriteTxRx()` |
| `syncReadTx()` | `syncReadTx()` |
| `syncReadRx()` | `syncReadRx()` |
| `syncWriteTxOnly()` | `syncWriteTxOnly()` |

### Helper Method Mapping

| Python Method | C++ Method |
|--------------|------------|
| `sts_makeword()` | `makeWord()` |
| `sts_makedword()` | `makeDWord()` |
| `sts_lobyte()` | `lobyte()` |
| `sts_hibyte()` | `hibyte()` |
| `sts_loword()` | `loword()` |
| `sts_hiword()` | `hiword()` |
| `sts_tohost()` | `toHost()` |
| `sts_toscs()` | `toScs()` |
| `sts_getend()` | `getEnd()` |
| `sts_setend()` | `setEnd()` |

## Key Implementation Details

### Return Value Differences

Some Python methods return inconsistent types. The C++ version normalizes these:

```python
# Python (inconsistent)
def StopServo(self, sts_id):
    # Returns True or None
    
def StartServo(self, sts_id):
    # Returns (comm, error) tuple!

def SetMode(self, sts_id, mode):
    # Returns (comm, error) tuple!
```

```cpp
// C++ (consistent)
bool stopServo(uint8_t sts_id);   // Returns bool
bool startServo(uint8_t sts_id);  // Returns bool
bool setMode(uint8_t sts_id, uint8_t mode);  // Returns bool
```

### Error Handling Normalization

```python
# Python: Returns None on error
def ReadPosition(self, sts_id):
    if comm == 0 and error == 0:
        return position
    else:
        return None
```

```cpp
// C++: Returns std::nullopt on error
std::optional<uint16_t> readPosition(uint8_t sts_id) {
    if (comm == COMM_SUCCESS && error == 0) {
        return position;
    }
    return std::nullopt;
}
```

## Adding New Features

1. Declare the method in the appropriate header file
2. Implement in the corresponding `.cpp` file
3. Add Doxygen documentation comment
4. If user-facing, add an example
5. Update the documentation
6. Verify the build compiles without warnings
