# Installation

This document provides step-by-step instructions for building, installing, integrating, and troubleshooting the ST3215 C++ library.

## Prerequisites

### Operating System

| OS | Status | Notes |
|----|--------|-------|
| Linux (Ubuntu, Debian, Fedora) | ✅ Supported | Primary platform |
| macOS | ✅ Supported | Same POSIX APIs |
| Windows | ⏳ Planned | Future support via Win32 API |

### Required Software

| Software | Minimum Version | Purpose |
|----------|----------------|---------|
| C++ compiler | GCC 7+ or Clang 5+ | Compilation |
| CMake | 3.10+ | Build system |
| Git | Any | Cloning the repository |
| Make or Ninja | Any | Build driver |

### Install Prerequisites

**Ubuntu / Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

**Fedora / RHEL / CentOS:**
```bash
sudo dnf install gcc-c++ cmake git make
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake git
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

---

## Building from Source

### Step 1: Clone the Repository

```bash
git clone https://github.com/sinayanaik/cpp_ST3215.git
cd cpp_ST3215
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
cmake ..
```

**With options:**
```bash
# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Without examples
cmake -DBUILD_EXAMPLES=OFF ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/opt/st3215 ..

# Combined
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
```

### Step 4: Build

```bash
cmake --build .
```

**Parallel build (faster):**
```bash
cmake --build . -- -j$(nproc)
```

### Step 5: Verify Build

After a successful build, you should see:

```
[100%] Built target read_telemetry
```

Built files:
```
build/
├── libst3215.so          # Shared library
├── libst3215.a           # Static library
└── examples/
    ├── ping_servo        # Example: ping a servo
    ├── list_servos       # Example: scan for servos
    ├── move_servo        # Example: move a servo
    └── read_telemetry    # Example: read sensor data
```

---

## System-Wide Installation

### Install

```bash
cd build
sudo cmake --install .
```

This installs:
- Headers → `/usr/local/include/st3215/`
- Libraries → `/usr/local/lib/`
- CMake config → `/usr/local/lib/cmake/ST3215/`

### Update Library Cache

After installing shared libraries:

```bash
sudo ldconfig
```

### Verify Installation

```bash
# Check headers
ls /usr/local/include/st3215/

# Check libraries
ls /usr/local/lib/libst3215*

# Check CMake config
ls /usr/local/lib/cmake/ST3215/
```

### Uninstall

```bash
cd build
sudo xargs rm < install_manifest.txt
```

---

## Integrating into Your Project

### Method 1: CMake `find_package` (Recommended)

After system-wide installation, add to your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyRobotApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ST3215 REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ST3215::st3215)
```

### Method 2: CMake `add_subdirectory`

Include the library as a subdirectory (no installation needed):

```
my_project/
├── CMakeLists.txt
├── main.cpp
└── cpp_ST3215/          # Clone or copy here
    ├── CMakeLists.txt
    ├── include/
    └── src/
```

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyRobotApp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(cpp_ST3215)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE st3215)
```

### Method 3: Manual Compilation

```bash
# Compile with shared library
g++ -std=c++17 main.cpp -o my_app -lst3215 -pthread

# Compile with static library (include path needed if not installed)
g++ -std=c++17 -I/path/to/cpp_ST3215/include main.cpp \
    /path/to/build/libst3215.a -o my_app -pthread
```

### Method 4: Include Sources Directly

Copy the `include/` and `src/` directories into your project and compile everything together:

```bash
g++ -std=c++17 -Iinclude \
    src/port_handler.cpp \
    src/protocol_packet_handler.cpp \
    src/st3215.cpp \
    src/group_sync_write.cpp \
    src/group_sync_read.cpp \
    main.cpp \
    -o my_app -pthread
```

---

## Hardware Setup

### Connecting Servos

1. **Power supply:** Connect 6-12V DC power supply to the servo's VCC and GND pins
2. **Serial adapter:** Connect a USB-to-TTL serial adapter to the servo's DATA pin
3. **Common ground:** Ensure the adapter and servo share a common ground

```
  USB Port          USB-Serial Adapter       ST3215 Servo(s)
  ┌─────┐           ┌───────────┐           ┌────────────┐
  │     ├───USB────►│           ├───DATA───►│            │
  │ PC  │           │  CH340 /  │           │  Servo #1  │
  │     │           │  CP2102   ├───GND────►│            │
  └─────┘           └───────────┘           └──────┬─────┘
                                                   │ Daisy-chain
                         Power Supply              ▼
                    ┌────────────────┐       ┌────────────┐
                    │  6-12V DC      ├──VCC─►│  Servo #2  │
                    │                ├──GND─►│            │
                    └────────────────┘       └────────────┘
```

### Finding the Serial Port

**Linux:**
```bash
# List USB serial ports
ls /dev/ttyUSB*
ls /dev/ttyACM*

# Watch for new devices
dmesg | tail -20
# or
sudo dmesg --follow
# Then plug in the adapter
```

**macOS:**
```bash
ls /dev/tty.usbserial*
ls /dev/cu.usbserial*
```

### Serial Port Permissions

**Option 1: Add user to dialout group (recommended)**
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for the change to take effect
```

**Option 2: Udev rule (persistent, device-specific)**
```bash
# Find your adapter's vendor/product ID
lsusb
# Example output: Bus 001 Device 003: ID 1a86:7523 QinHeng Electronics CH340 serial converter

# Create udev rule
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE="0666"' | \
    sudo tee /etc/udev/rules.d/99-st3215.rules

# Reload rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

**Option 3: Run with sudo (not recommended for production)**
```bash
sudo ./my_app
```

---

## Running Examples

After building, test with the example programs:

```bash
# From the build directory:

# 1. Ping a servo (check if servo ID 1 is responding)
./examples/ping_servo /dev/ttyUSB0 1

# 2. List all servos on the bus
./examples/list_servos /dev/ttyUSB0

# 3. Move servo 1 to center position (2048)
./examples/move_servo /dev/ttyUSB0 1 2048

# 4. Read all telemetry from servo 1
./examples/read_telemetry /dev/ttyUSB0 1
```

---

## Troubleshooting

### Build Errors

**Error: `cmake not found`**
```bash
sudo apt-get install cmake
```

**Error: `Could NOT find Threads`**
```bash
sudo apt-get install libpthread-stubs0-dev
```

**Error: `no member named 'optional' in namespace 'std'`**

Your compiler doesn't support C++17. Upgrade:
```bash
sudo apt-get install g++-9
cmake -DCMAKE_CXX_COMPILER=g++-9 ..
```

### Runtime Errors

**Error: `Could not open port: /dev/ttyUSB0`**

1. Check the port exists: `ls /dev/ttyUSB*`
2. Check permissions: `ls -la /dev/ttyUSB0`
3. Add user to dialout group: `sudo usermod -a -G dialout $USER`
4. Check if port is in use: `lsof /dev/ttyUSB0`

**Error: `No servos found`**

1. Verify power supply is connected and adequate (6-12V)
2. Check wiring connections
3. Ensure baudrate matches servo configuration (default: 1,000,000)
4. Try pinging a specific ID if you know it

**Error: `Permission denied`**

```bash
sudo chmod 666 /dev/ttyUSB0
# Or (better):
sudo usermod -a -G dialout $USER
```

### Library Not Found at Runtime

If you get a shared library error when running your program:

```
error while loading shared libraries: libst3215.so: cannot open shared object file
```

Fix:
```bash
sudo ldconfig
# Or specify the library path:
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

---

## Quick Verification Program

Create `test_install.cpp`:

```cpp
#include "st3215/st3215.h"
#include <iostream>

int main() {
    std::cout << "ST3215 library loaded successfully!" << std::endl;
    std::cout << "Protocol version: 1.0" << std::endl;
    std::cout << "Position range: 0-4095" << std::endl;
    std::cout << "Max speed: 3400 step/s" << std::endl;
    std::cout << "Default baudrate: 1,000,000 bps" << std::endl;

    // Test compilation of key types
    st3215::ST3215* ptr = nullptr;
    (void)ptr;

    std::cout << "All types compiled successfully!" << std::endl;
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++17 test_install.cpp -o test_install -lst3215 -pthread
./test_install
```

Expected output:
```
ST3215 library loaded successfully!
Protocol version: 1.0
Position range: 0-4095
Max speed: 3400 step/s
Default baudrate: 1,000,000 bps
All types compiled successfully!
```
