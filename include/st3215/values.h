#ifndef ST3215_VALUES_H
#define ST3215_VALUES_H

#include <cstdint>

namespace st3215 {

// Communication Settings
constexpr uint32_t DEFAULT_BAUDRATE = 1000000;
constexpr uint32_t LATENCY_TIMER = 50;

// Packet Limits
constexpr size_t TXPACKET_MAX_LEN = 250;
constexpr size_t RXPACKET_MAX_LEN = 250;

// Position Limits
constexpr uint16_t MIN_POSITION = 0;
constexpr uint16_t MAX_POSITION = 4095;

// Speed and Correction Limits
constexpr uint16_t MAX_SPEED = 3400;
constexpr uint16_t MAX_CORRECTION = 2047;

// Protocol Packet Structure
constexpr uint8_t PKT_HEADER_0 = 0;
constexpr uint8_t PKT_HEADER_1 = 1;
constexpr uint8_t PKT_ID = 2;
constexpr uint8_t PKT_LENGTH = 3;
constexpr uint8_t PKT_INSTRUCTION = 4;
constexpr uint8_t PKT_ERROR = 4;
constexpr uint8_t PKT_PARAMETER0 = 5;

// Protocol Error Bits
constexpr uint8_t ERRBIT_VOLTAGE = 1;
constexpr uint8_t ERRBIT_ANGLE = 2;
constexpr uint8_t ERRBIT_OVERHEAT = 4;
constexpr uint8_t ERRBIT_OVERELE = 8;
constexpr uint8_t ERRBIT_OVERLOAD = 32;

// Special IDs
constexpr uint8_t BROADCAST_ID = 0xFE;  // 254
constexpr uint8_t MAX_ID = 0xFC;        // 252
constexpr uint8_t STS_END = 0;

// Instructions for STS Protocol
constexpr uint8_t INST_PING = 1;
constexpr uint8_t INST_READ = 2;
constexpr uint8_t INST_WRITE = 3;
constexpr uint8_t INST_REG_WRITE = 4;
constexpr uint8_t INST_ACTION = 5;
constexpr uint8_t INST_SYNC_WRITE = 131;  // 0x83
constexpr uint8_t INST_SYNC_READ = 130;   // 0x82

// Communication Results
constexpr int COMM_SUCCESS = 0;         // tx or rx packet communication success
constexpr int COMM_PORT_BUSY = -1;      // Port is busy (in use)
constexpr int COMM_TX_FAIL = -2;        // Failed transmit instruction packet
constexpr int COMM_RX_FAIL = -3;        // Failed get status packet
constexpr int COMM_TX_ERROR = -4;       // Incorrect instruction packet
constexpr int COMM_RX_WAITING = -5;     // Now receiving status packet
constexpr int COMM_RX_TIMEOUT = -6;     // There is no status packet
constexpr int COMM_RX_CORRUPT = -7;     // Incorrect status packet
constexpr int COMM_NOT_AVAILABLE = -9;  // Protocol does not support this function

// Bus Speed Settings
constexpr uint8_t STS_1M = 0;
constexpr uint8_t STS_0_5M = 1;
constexpr uint8_t STS_250K = 2;
constexpr uint8_t STS_128K = 3;
constexpr uint8_t STS_115200 = 4;
constexpr uint8_t STS_76800 = 5;
constexpr uint8_t STS_57600 = 6;
constexpr uint8_t STS_38400 = 7;

// EPROM Read-Only Registers
constexpr uint8_t STS_MODEL_L = 3;
constexpr uint8_t STS_MODEL_H = 4;

// EPROM Read-Write Registers
constexpr uint8_t STS_ID = 5;
constexpr uint8_t STS_BAUD_RATE = 6;
constexpr uint8_t STS_MIN_ANGLE_LIMIT_L = 9;
constexpr uint8_t STS_MIN_ANGLE_LIMIT_H = 10;
constexpr uint8_t STS_MAX_ANGLE_LIMIT_L = 11;
constexpr uint8_t STS_MAX_ANGLE_LIMIT_H = 12;
constexpr uint8_t STS_CW_DEAD = 26;
constexpr uint8_t STS_CCW_DEAD = 27;
constexpr uint8_t STS_OFS_L = 31;
constexpr uint8_t STS_OFS_H = 32;
constexpr uint8_t STS_MODE = 33;

// SRAM Read-Write Registers
constexpr uint8_t STS_TORQUE_ENABLE = 40;
constexpr uint8_t STS_ACC = 41;
constexpr uint8_t STS_GOAL_POSITION_L = 42;
constexpr uint8_t STS_GOAL_POSITION_H = 43;
constexpr uint8_t STS_GOAL_TIME_L = 44;
constexpr uint8_t STS_GOAL_TIME_H = 45;
constexpr uint8_t STS_GOAL_SPEED_L = 46;
constexpr uint8_t STS_GOAL_SPEED_H = 47;
constexpr uint8_t STS_LOCK = 55;

// SRAM Read-Only Registers
constexpr uint8_t STS_PRESENT_POSITION_L = 56;
constexpr uint8_t STS_PRESENT_POSITION_H = 57;
constexpr uint8_t STS_PRESENT_SPEED_L = 58;
constexpr uint8_t STS_PRESENT_SPEED_H = 59;
constexpr uint8_t STS_PRESENT_LOAD_L = 60;
constexpr uint8_t STS_PRESENT_LOAD_H = 61;
constexpr uint8_t STS_PRESENT_VOLTAGE = 62;
constexpr uint8_t STS_PRESENT_TEMPERATURE = 63;
constexpr uint8_t STS_STATUS = 65;
constexpr uint8_t STS_MOVING = 66;
constexpr uint8_t STS_PRESENT_CURRENT_L = 69;
constexpr uint8_t STS_PRESENT_CURRENT_H = 70;

}  // namespace st3215

#endif  // ST3215_VALUES_H
