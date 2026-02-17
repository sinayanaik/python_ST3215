#include "st3215/st3215.h"
#include <iostream>
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
        
        std::cout << "Pinging servo " << servo_id << "..." << std::endl;
        
        if (servo.pingServo(servo_id)) {
            std::cout << "Servo " << servo_id << " is responding!" << std::endl;
            return 0;
        } else {
            std::cout << "Servo " << servo_id << " is not responding." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
