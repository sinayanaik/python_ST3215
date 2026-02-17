#include "st3215/st3215.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <servo_id> <position>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /dev/ttyUSB0 1 2048" << std::endl;
        std::cerr << "Position range: 0-4095" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    int servo_id = std::atoi(argv[2]);
    int position = std::atoi(argv[3]);

    if (position < 0 || position > 4095) {
        std::cerr << "Error: Position must be between 0 and 4095" << std::endl;
        return 1;
    }

    try {
        st3215::ST3215 servo(port);
        
        std::cout << "Moving servo " << servo_id << " to position " << position << "..." << std::endl;
        
        if (servo.moveTo(servo_id, position, 2400, 50, true)) {
            std::cout << "Servo moved successfully!" << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to move servo." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
