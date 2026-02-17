#include "st3215/st3215.h"
#include <thread>
#include <chrono>
#include <cmath>
#include <stdexcept>

namespace st3215 {

ST3215::ST3215(const std::string& device)
    : ProtocolPacketHandler(nullptr),
      port_handler_(std::make_unique<PortHandler>(device)) {
    
    if (!port_handler_->openPort()) {
        throw std::runtime_error("Could not open port: " + device);
    }
    
    // Update the base class to use our port handler
    ProtocolPacketHandler::port_handler_ = port_handler_.get();
}

ST3215::~ST3215() {
    if (port_handler_) {
        port_handler_->closePort();
    }
}

bool ST3215::pingServo(uint8_t sts_id) {
    uint16_t model;
    int comm;
    uint8_t error;
    std::tie(model, comm, error) = ping(sts_id);
    
    if (comm != COMM_SUCCESS || model == 0 || error != 0) {
        return false;
    }
    return true;
}

std::vector<uint8_t> ST3215::listServos() {
    std::vector<uint8_t> servos;
    for (int id = 0; id < 254; ++id) {
        if (pingServo(static_cast<uint8_t>(id))) {
            servos.push_back(static_cast<uint8_t>(id));
        }
    }
    return servos;
}

std::optional<double> ST3215::readLoad(uint8_t sts_id) {
    uint8_t load;
    int comm;
    uint8_t error;
    std::tie(load, comm, error) = read1ByteTxRx(sts_id, STS_PRESENT_LOAD_L);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return load * 0.1;
    }
    return std::nullopt;
}

std::optional<double> ST3215::readVoltage(uint8_t sts_id) {
    uint8_t voltage;
    int comm;
    uint8_t error;
    std::tie(voltage, comm, error) = read1ByteTxRx(sts_id, STS_PRESENT_VOLTAGE);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return voltage * 0.1;
    }
    return std::nullopt;
}

std::optional<double> ST3215::readCurrent(uint8_t sts_id) {
    uint8_t current;
    int comm;
    uint8_t error;
    std::tie(current, comm, error) = read1ByteTxRx(sts_id, STS_PRESENT_CURRENT_L);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return current * 6.5;
    }
    return std::nullopt;
}

std::optional<int> ST3215::readTemperature(uint8_t sts_id) {
    uint8_t temperature;
    int comm;
    uint8_t error;
    std::tie(temperature, comm, error) = read1ByteTxRx(sts_id, STS_PRESENT_TEMPERATURE);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return temperature;
    }
    return std::nullopt;
}

std::optional<uint8_t> ST3215::readAcceleration(uint8_t sts_id) {
    uint8_t acc;
    int comm;
    uint8_t error;
    std::tie(acc, comm, error) = read1ByteTxRx(sts_id, STS_ACC);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return acc;
    }
    return std::nullopt;
}

std::optional<uint8_t> ST3215::readMode(uint8_t sts_id) {
    uint8_t mode;
    int comm;
    uint8_t error;
    std::tie(mode, comm, error) = read1ByteTxRx(sts_id, STS_MODE);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return mode;
    }
    return std::nullopt;
}

std::optional<int16_t> ST3215::readCorrection(uint8_t sts_id) {
    uint16_t correction;
    int comm;
    uint8_t error;
    std::tie(correction, comm, error) = read2ByteTxRx(sts_id, STS_OFS_L);
    
    if (comm == COMM_SUCCESS && error == 0) {
        uint16_t mask = 0x07FF;
        int16_t bits = correction & mask;
        if ((correction & 0x0800) != 0) {
            bits = -1 * (bits & 0x7FF);
        }
        return bits;
    }
    return std::nullopt;
}

std::optional<bool> ST3215::isMoving(uint8_t sts_id) {
    uint8_t moving;
    int comm;
    uint8_t error;
    std::tie(moving, comm, error) = read1ByteTxRx(sts_id, STS_MOVING);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return static_cast<bool>(moving);
    }
    return std::nullopt;
}

std::optional<uint16_t> ST3215::readPosition(uint8_t sts_id) {
    uint16_t position;
    int comm;
    uint8_t error;
    std::tie(position, comm, error) = read2ByteTxRx(sts_id, STS_PRESENT_POSITION_L);
    
    if (comm == COMM_SUCCESS && error == 0) {
        return position;
    }
    return std::nullopt;
}

std::tuple<int16_t, int, uint8_t> ST3215::readSpeed(uint8_t sts_id) {
    uint16_t sts_present_speed;
    int sts_comm_result;
    uint8_t sts_error;
    std::tie(sts_present_speed, sts_comm_result, sts_error) = read2ByteTxRx(sts_id, STS_PRESENT_SPEED_L);
    int16_t speed = toHost(sts_present_speed, 15);
    return std::make_tuple(speed, sts_comm_result, sts_error);
}

std::optional<std::map<std::string, bool>> ST3215::readStatus(uint8_t sts_id) {
    const std::vector<std::string> status_bits = {
        "Voltage", "Sensor", "Temperature", "Current", "Angle", "Overload"
    };
    
    std::map<std::string, bool> status;
    
    uint8_t status_byte;
    int comm;
    uint8_t error;
    std::tie(status_byte, comm, error) = read1ByteTxRx(sts_id, STS_STATUS);
    
    if (comm != COMM_SUCCESS || error != 0) {
        return std::nullopt;
    }
    
    for (size_t i = 0; i < status_bits.size(); ++i) {
        if (status_byte & (1 << i)) {
            status[status_bits[i]] = false;
        } else {
            status[status_bits[i]] = true;
        }
    }
    
    return status;
}

bool ST3215::setAcceleration(uint8_t sts_id, uint8_t acc) {
    std::vector<uint8_t> txpacket = {acc};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_ACC, 1, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::setSpeed(uint8_t sts_id, uint16_t speed) {
    std::vector<uint8_t> txpacket = {lobyte(speed), hibyte(speed)};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_GOAL_SPEED_L, 2, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::stopServo(uint8_t sts_id) {
    std::vector<uint8_t> txpacket = {0};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_TORQUE_ENABLE, 1, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::startServo(uint8_t sts_id) {
    std::vector<uint8_t> txpacket = {1};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_TORQUE_ENABLE, 1, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::setMode(uint8_t sts_id, uint8_t mode) {
    std::vector<uint8_t> txpacket = {mode};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_MODE, 1, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::correctPosition(uint8_t sts_id, int16_t correction) {
    uint16_t corr = std::abs(correction);
    if (corr > MAX_CORRECTION) {
        corr = MAX_CORRECTION;
    }
    
    std::vector<uint8_t> txpacket = {lobyte(corr), hibyte(corr)};
    
    if (correction < 0) {
        txpacket[1] |= (1 << 3);
    }
    
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_OFS_L, 2, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::rotate(uint8_t sts_id, int16_t speed) {
    if (!setMode(sts_id, 1)) {
        return false;
    }
    
    uint16_t abs_speed = std::abs(speed);
    if (abs_speed > MAX_SPEED) {
        abs_speed = MAX_SPEED;
    }
    
    std::vector<uint8_t> txpacket = {lobyte(abs_speed), hibyte(abs_speed)};
    
    if (speed < 0) {
        txpacket[1] |= (1 << 7);
    }
    
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_GOAL_SPEED_L, 2, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

bool ST3215::moveTo(uint8_t sts_id, uint16_t position, uint16_t speed, uint8_t acc, bool wait) {
    if (!setMode(sts_id, 0) || !setAcceleration(sts_id, acc) || !setSpeed(sts_id, speed)) {
        return false;
    }
    
    auto curr_pos = readPosition(sts_id);
    
    if (!writePosition(sts_id, position)) {
        return false;
    }
    
    if (wait && curr_pos.has_value()) {
        uint16_t distance = std::abs(static_cast<int>(position) - static_cast<int>(curr_pos.value()));
        
        double time_to_speed = static_cast<double>(speed) / (acc * 100.0);
        double distance_acc = 0.5 * (acc * 100.0) * time_to_speed * time_to_speed;
        
        double time_wait;
        if (distance_acc >= distance) {
            time_wait = std::sqrt(2.0 * distance / acc);
        } else {
            double remain_distance = distance - distance_acc;
            time_wait = time_to_speed + (remain_distance / speed);
        }
        
        std::this_thread::sleep_for(std::chrono::duration<double>(time_wait));
    }
    
    return true;
}

bool ST3215::writePosition(uint8_t sts_id, uint16_t position) {
    std::vector<uint8_t> txpacket = {lobyte(position), hibyte(position)};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_GOAL_POSITION_L, 2, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

int ST3215::lockEprom(uint8_t sts_id) {
    return write1ByteTxOnly(sts_id, STS_LOCK, 1);
}

int ST3215::unlockEprom(uint8_t sts_id) {
    return write1ByteTxOnly(sts_id, STS_LOCK, 0);
}

std::string ST3215::changeId(uint8_t sts_id, uint8_t new_id) {
    if (new_id > 253) {
        return "new_id is not between 0 and 253";
    }
    
    if (!pingServo(sts_id)) {
        return "Could not find servo: " + std::to_string(sts_id);
    }
    
    if (unlockEprom(sts_id) != COMM_SUCCESS) {
        return "Could not unlock Eprom";
    }
    
    if (write1ByteTxOnly(sts_id, STS_ID, new_id) != COMM_SUCCESS) {
        return "Could not change Servo ID";
    }
    
    lockEprom(sts_id);
    return "";
}

std::string ST3215::changeBaudrate(uint8_t sts_id, uint8_t new_baudrate) {
    if (new_baudrate > 7) {
        return "baudrate is not valid";
    }
    
    if (!pingServo(sts_id)) {
        return "Could not find servo: " + std::to_string(sts_id);
    }
    
    if (unlockEprom(sts_id) != COMM_SUCCESS) {
        return "Could not unlock Eprom";
    }
    
    if (write1ByteTxOnly(sts_id, STS_BAUD_RATE, new_baudrate) != COMM_SUCCESS) {
        return "Could not change Servo Baudrate";
    }
    
    lockEprom(sts_id);
    return "";
}

bool ST3215::defineMiddle(uint8_t sts_id) {
    std::vector<uint8_t> txpacket = {128};
    int comm;
    uint8_t error;
    std::tie(comm, error) = writeTxRx(sts_id, STS_TORQUE_ENABLE, 1, txpacket);
    return (comm == COMM_SUCCESS && error == 0);
}

std::optional<uint16_t> ST3215::getBlockPosition(uint8_t sts_id) {
    int stop_matches = 0;
    
    while (true) {
        auto moving = isMoving(sts_id);
        if (!moving.has_value()) {
            setMode(sts_id, 0);
            stopServo(sts_id);
            return std::nullopt;
        }
        
        if (!moving.value()) {
            auto position = readPosition(sts_id);
            setMode(sts_id, 0);
            stopServo(sts_id);
            
            if (!position.has_value()) {
                return std::nullopt;
            }
            
            stop_matches++;
            if (stop_matches > 4) {
                return position;
            }
        } else {
            stop_matches = 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

std::tuple<std::optional<uint16_t>, std::optional<uint16_t>> ST3215::tareServo(uint8_t sts_id) {
    if (!correctPosition(sts_id, 0)) {
        return std::make_tuple(std::nullopt, std::nullopt);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    setAcceleration(sts_id, 100);
    rotate(sts_id, -250);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto min_position = getBlockPosition(sts_id);
    
    rotate(sts_id, 250);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto max_position = getBlockPosition(sts_id);
    
    if (min_position.has_value() && max_position.has_value()) {
        uint16_t min_pos = min_position.value();
        uint16_t max_pos = max_position.value();
        
        // Calculate middle of the path
        int distance;
        if (min_pos >= max_pos) {
            distance = static_cast<int>((MAX_POSITION - min_pos + max_pos) / 2);
        } else {
            distance = static_cast<int>((max_pos - min_pos) / 2);
        }
        
        // Calculate correction
        int16_t corr;
        if (min_pos > MAX_POSITION / 2) {
            corr = min_pos - MAX_POSITION - 1;
        } else {
            corr = min_pos;
        }
        
        if (correctPosition(sts_id, corr)) {
            min_pos = 0;
            max_pos = distance * 2;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            moveTo(sts_id, distance);
        }
        
        return std::make_tuple(std::optional<uint16_t>(min_pos), std::optional<uint16_t>(max_pos));
    }
    
    return std::make_tuple(std::nullopt, std::nullopt);
}

}  // namespace st3215
