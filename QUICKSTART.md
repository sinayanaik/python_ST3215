# Quick Start Guide - ST3215 C++ Library

Get up and running with ST3215 servos in minutes!

## Installation

### Option 1: Build and Install System-Wide

```bash
git clone https://github.com/sinayanaik/python_ST3215.git
cd python_ST3215
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Option 2: Use Locally

```bash
git clone https://github.com/sinayanaik/python_ST3215.git
cd python_ST3215
mkdir build && cd build
cmake ..
cmake --build .
# Use examples from build/examples/
```

## Hardware Setup

1. **Connect ST3215 servo to USB-Serial adapter**
   - VCC → Power supply (typically 6-12V)
   - GND → Common ground
   - Data → TX/RX (serial data line)

2. **Find your serial port**
   ```bash
   # Linux
   ls /dev/ttyUSB* /dev/ttyACM*
   
   # Usually /dev/ttyUSB0
   ```

3. **Set permissions** (Linux)
   ```bash
   sudo usermod -a -G dialout $USER
   # Then logout and login
   ```

## First Program - List Servos

Create `test.cpp`:

```cpp
#include "st3215/st3215.h"
#include <iostream>

int main() {
    try {
        // Open serial port
        st3215::ST3215 servo("/dev/ttyUSB0");
        
        // Find all servos
        std::cout << "Scanning for servos..." << std::endl;
        auto servos = servo.listServos();
        
        std::cout << "Found " << servos.size() << " servo(s)" << std::endl;
        for (auto id : servos) {
            std::cout << "  Servo ID: " << (int)id << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

**Compile and run:**

```bash
# If installed system-wide
g++ -std=c++17 test.cpp -o test -lst3215 -pthread
./test

# Or use built examples
./build/examples/list_servos /dev/ttyUSB0
```

## Common Tasks

### 1. Ping a Servo

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

if (servo.pingServo(1)) {
    std::cout << "Servo 1 is responding!" << std::endl;
}
```

### 2. Move to Position

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

// Move servo 1 to center position (2048)
servo.moveTo(1, 2048);

// Move with custom speed and wait for completion
servo.moveTo(1, 3000, 1500, 100, true);
```

### 3. Read Sensor Data

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

auto position = servo.readPosition(1);
auto temperature = servo.readTemperature(1);
auto voltage = servo.readVoltage(1);

if (position.has_value()) {
    std::cout << "Position: " << position.value() << std::endl;
}
if (temperature.has_value()) {
    std::cout << "Temperature: " << temperature.value() << "°C" << std::endl;
}
if (voltage.has_value()) {
    std::cout << "Voltage: " << voltage.value() << "V" << std::endl;
}
```

### 4. Continuous Rotation

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

// Rotate clockwise at 500 step/s
servo.rotate(1, 500);

// Wait a bit
std::this_thread::sleep_for(std::chrono::seconds(2));

// Stop
servo.stopServo(1);
```

### 5. Change Servo ID

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

// Change servo ID from 1 to 5
auto result = servo.changeId(1, 5);
if (result.empty()) {
    std::cout << "ID changed successfully!" << std::endl;
} else {
    std::cerr << "Error: " << result << std::endl;
}
```

## Using CMake in Your Project

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyServoApp)

set(CMAKE_CXX_STANDARD 17)

# Find ST3215 library
find_package(ST3215 REQUIRED)

# Create executable
add_executable(my_app main.cpp)

# Link against ST3215
target_link_libraries(my_app PRIVATE ST3215::st3215)
```

## Error Handling

Always check return values:

```cpp
st3215::ST3215 servo("/dev/ttyUSB0");

// Method 1: Using optional
auto pos = servo.readPosition(1);
if (pos.has_value()) {
    std::cout << "Position: " << pos.value() << std::endl;
} else {
    std::cerr << "Failed to read position" << std::endl;
}

// Method 2: Using boolean return
if (servo.moveTo(1, 2048)) {
    std::cout << "Move command sent" << std::endl;
} else {
    std::cerr << "Failed to send move command" << std::endl;
}

// Method 3: Try-catch for exceptions
try {
    st3215::ST3215 servo("/dev/INVALID_PORT");
} catch (const std::exception& e) {
    std::cerr << "Failed to open port: " << e.what() << std::endl;
}
```

## Position Range

ST3215 servos have a position range of **0-4095**:
- **0**: Minimum position
- **2048**: Center position  
- **4095**: Maximum position

```cpp
servo.moveTo(1, 0);     // Move to minimum
servo.moveTo(1, 2048);  // Move to center
servo.moveTo(1, 4095);  // Move to maximum
```

## Speed and Acceleration

- **Speed**: 0-3400 (step/s)
- **Acceleration**: 0-254 (unit: 100 step/s²)

```cpp
servo.setSpeed(1, 2400);       // Set speed to 2400 step/s
servo.setAcceleration(1, 50);  // Set acceleration to 5000 step/s²
```

## Operating Modes

```cpp
// Mode 0: Position Mode (default)
servo.setMode(1, 0);
servo.moveTo(1, 2048);

// Mode 1: Constant Speed Mode
servo.setMode(1, 1);
servo.rotate(1, 500);

// Mode 2: PWM Mode
servo.setMode(1, 2);

// Mode 3: Step Servo Mode
servo.setMode(1, 3);
```

## Troubleshooting

### "Could not open port"

1. Check port name: `ls /dev/ttyUSB*`
2. Check permissions: Add user to dialout group
3. Check if port is in use: `lsof /dev/ttyUSB0`

### "No servos found"

1. Check physical connections
2. Verify power supply
3. Try different baud rate
4. Check servo firmware version

### Compilation errors

```bash
# Make sure you have C++17
g++ --version  # Should be 7+ for GCC

# Install dependencies
sudo apt-get install build-essential cmake

# Check library installation
pkg-config --cflags --libs st3215
```

## Next Steps

1. Read the full [README.md](README.md) for complete API documentation
2. Check [TECHNICAL_COMPARISON.md](TECHNICAL_COMPARISON.md) for performance details
3. See [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute
4. Explore examples in `examples/` directory
5. Join discussions on GitHub for support

## Complete Example - Servo Control Loop

```cpp
#include "st3215/st3215.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        st3215::ST3215 servo("/dev/ttyUSB0");
        
        // Find servos
        auto servos = servo.listServos();
        if (servos.empty()) {
            std::cerr << "No servos found!" << std::endl;
            return 1;
        }
        
        uint8_t servo_id = servos[0];
        std::cout << "Using servo " << (int)servo_id << std::endl;
        
        // Move to center
        std::cout << "Moving to center..." << std::endl;
        servo.moveTo(servo_id, 2048, 2400, 50, true);
        
        // Read telemetry
        auto pos = servo.readPosition(servo_id);
        auto temp = servo.readTemperature(servo_id);
        auto volt = servo.readVoltage(servo_id);
        
        std::cout << "Position: " << (pos.has_value() ? pos.value() : 0) << std::endl;
        std::cout << "Temperature: " << (temp.has_value() ? temp.value() : 0) << "°C" << std::endl;
        std::cout << "Voltage: " << (volt.has_value() ? volt.value() : 0.0) << "V" << std::endl;
        
        // Sweep motion
        std::cout << "Starting sweep..." << std::endl;
        for (int i = 0; i < 3; ++i) {
            servo.moveTo(servo_id, 1024, 2000, 50, true);
            servo.moveTo(servo_id, 3072, 2000, 50, true);
        }
        
        // Return to center
        servo.moveTo(servo_id, 2048, 2400, 50, true);
        std::cout << "Done!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

Compile and run:
```bash
g++ -std=c++17 example.cpp -o example -lst3215 -pthread
./example
```

---

**Happy coding with ST3215 servos! 🤖**
