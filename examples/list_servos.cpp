#include "st3215/st3215.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /dev/ttyUSB0" << std::endl;
        return 1;
    }

    std::string port = argv[1];

    try {
        st3215::ST3215 servo(port);
        
        std::cout << "Scanning for servos..." << std::endl;
        auto servos = servo.listServos();
        
        if (servos.empty()) {
            std::cout << "No servos found." << std::endl;
            return 0;
        }
        
        std::cout << "Found " << servos.size() << " servo(s):" << std::endl;
        for (auto id : servos) {
            std::cout << "  - Servo ID: " << static_cast<int>(id) << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
