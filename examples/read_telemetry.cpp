#include "st3215/st3215.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <servo_id>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /dev/ttyUSB0 1" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    int servo_id = std::atoi(argv[2]);

    try {
        st3215::ST3215 servo(port);
        
        std::cout << "Reading telemetry from servo " << servo_id << "..." << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        
        // Read position
        auto position = servo.readPosition(servo_id);
        if (position.has_value()) {
            std::cout << "Position: " << position.value() << " (0-4095)" << std::endl;
        } else {
            std::cout << "Position: Failed to read" << std::endl;
        }
        
        // Read voltage
        auto voltage = servo.readVoltage(servo_id);
        if (voltage.has_value()) {
            std::cout << "Voltage: " << voltage.value() << " V" << std::endl;
        } else {
            std::cout << "Voltage: Failed to read" << std::endl;
        }
        
        // Read current
        auto current = servo.readCurrent(servo_id);
        if (current.has_value()) {
            std::cout << "Current: " << current.value() << " mA" << std::endl;
        } else {
            std::cout << "Current: Failed to read" << std::endl;
        }
        
        // Read temperature
        auto temperature = servo.readTemperature(servo_id);
        if (temperature.has_value()) {
            std::cout << "Temperature: " << temperature.value() << " °C" << std::endl;
        } else {
            std::cout << "Temperature: Failed to read" << std::endl;
        }
        
        // Read load
        auto load = servo.readLoad(servo_id);
        if (load.has_value()) {
            std::cout << "Load: " << load.value() << " %" << std::endl;
        } else {
            std::cout << "Load: Failed to read" << std::endl;
        }
        
        // Check if moving
        auto moving = servo.isMoving(servo_id);
        if (moving.has_value()) {
            std::cout << "Moving: " << (moving.value() ? "Yes" : "No") << std::endl;
        } else {
            std::cout << "Moving: Failed to read" << std::endl;
        }
        
        // Read status
        auto status = servo.readStatus(servo_id);
        if (status.has_value()) {
            std::cout << "Status:" << std::endl;
            for (const auto& [key, value] : status.value()) {
                std::cout << "  " << key << ": " << (value ? "OK" : "ERROR") << std::endl;
            }
        } else {
            std::cout << "Status: Failed to read" << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
