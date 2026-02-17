cmake_minimum_required(VERSION 3.10)

# Example: Ping servo
add_executable(ping_servo ping_servo.cpp)
target_link_libraries(ping_servo PRIVATE st3215)

# Example: List servos
add_executable(list_servos list_servos.cpp)
target_link_libraries(list_servos PRIVATE st3215)

# Example: Move servo
add_executable(move_servo move_servo.cpp)
target_link_libraries(move_servo PRIVATE st3215)

# Example: Read telemetry
add_executable(read_telemetry read_telemetry.cpp)
target_link_libraries(read_telemetry PRIVATE st3215)
