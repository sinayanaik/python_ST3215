#include "st3215/protocol_packet_handler.h"
#include <algorithm>
#include <thread>
#include <chrono>

namespace st3215 {

ProtocolPacketHandler::ProtocolPacketHandler(PortHandler* port_handler)
    : port_handler_(port_handler), sts_end_(0) {
}

std::string ProtocolPacketHandler::getTxRxResult(int result) const {
    switch (result) {
        case COMM_SUCCESS: return "[TxRxResult] Communication success!";
        case COMM_PORT_BUSY: return "[TxRxResult] Port is in use!";
        case COMM_TX_FAIL: return "[TxRxResult] Failed transmit instruction packet!";
        case COMM_RX_FAIL: return "[TxRxResult] Failed get status packet from device!";
        case COMM_TX_ERROR: return "[TxRxResult] Incorrect instruction packet!";
        case COMM_RX_WAITING: return "[TxRxResult] Now receiving status packet!";
        case COMM_RX_TIMEOUT: return "[TxRxResult] There is no status packet!";
        case COMM_RX_CORRUPT: return "[TxRxResult] Incorrect status packet!";
        case COMM_NOT_AVAILABLE: return "[TxRxResult] Protocol does not support this function!";
        default: return "";
    }
}

std::string ProtocolPacketHandler::getRxPacketError(uint8_t error) const {
    if (error & ERRBIT_VOLTAGE) return "[ServoStatus] Input voltage error!";
    if (error & ERRBIT_ANGLE) return "[ServoStatus] Angle sensor error!";
    if (error & ERRBIT_OVERHEAT) return "[ServoStatus] Overheat error!";
    if (error & ERRBIT_OVERELE) return "[ServoStatus] OverEle error!";
    if (error & ERRBIT_OVERLOAD) return "[ServoStatus] Overload error!";
    return "";
}

uint16_t ProtocolPacketHandler::makeWord(uint8_t a, uint8_t b) const {
    if (sts_end_ == 0) {
        return (a & 0xFF) | ((b & 0xFF) << 8);
    } else {
        return (b & 0xFF) | ((a & 0xFF) << 8);
    }
}

uint32_t ProtocolPacketHandler::makeDWord(uint16_t a, uint16_t b) const {
    return (a & 0xFFFF) | ((b & 0xFFFF) << 16);
}

uint8_t ProtocolPacketHandler::lobyte(uint16_t w) const {
    if (sts_end_ == 0) {
        return w & 0xFF;
    } else {
        return (w >> 8) & 0xFF;
    }
}

uint8_t ProtocolPacketHandler::hibyte(uint16_t w) const {
    if (sts_end_ == 0) {
        return (w >> 8) & 0xFF;
    } else {
        return w & 0xFF;
    }
}

int16_t ProtocolPacketHandler::toHost(uint16_t a, uint8_t b) const {
    if (a & (1 << b)) {
        return -(a & ~(1 << b));
    } else {
        return a;
    }
}

int ProtocolPacketHandler::txPacket(std::vector<uint8_t>& txpacket) {
    uint8_t checksum = 0;
    size_t total_packet_length = txpacket[PKT_LENGTH] + 4;  // 4: HEADER0 HEADER1 ID LENGTH

    if (port_handler_->isUsing()) {
        return COMM_PORT_BUSY;
    }
    port_handler_->setUsing(true);

    // Check max packet length
    if (total_packet_length > TXPACKET_MAX_LEN) {
        port_handler_->setUsing(false);
        return COMM_TX_ERROR;
    }

    // Make packet header
    txpacket[PKT_HEADER_0] = 0xFF;
    txpacket[PKT_HEADER_1] = 0xFF;

    // Add checksum
    for (size_t idx = 2; idx < total_packet_length - 1; ++idx) {
        checksum += txpacket[idx];
    }
    txpacket[total_packet_length - 1] = ~checksum & 0xFF;

    // Transmit packet
    port_handler_->clearPort();
    size_t written_packet_length = port_handler_->writePort(txpacket);
    if (total_packet_length != written_packet_length) {
        port_handler_->setUsing(false);
        return COMM_TX_FAIL;
    }

    return COMM_SUCCESS;
}

std::tuple<std::vector<uint8_t>, int> ProtocolPacketHandler::rxPacket() {
    std::vector<uint8_t> rxpacket;
    int result = COMM_TX_FAIL;
    uint8_t checksum = 0;
    size_t rx_length = 0;
    size_t wait_length = 6;  // Minimum length (HEADER0 HEADER1 ID LENGTH ERROR CHKSUM)

    while (true) {
        auto new_data = port_handler_->readPort(wait_length - rx_length);
        rxpacket.insert(rxpacket.end(), new_data.begin(), new_data.end());
        rx_length = rxpacket.size();

        if (rx_length >= wait_length) {
            // Find packet header
            size_t idx;
            for (idx = 0; idx < rx_length - 1; ++idx) {
                if (rxpacket[idx] == 0xFF && rxpacket[idx + 1] == 0xFF) {
                    break;
                }
            }

            if (idx == 0) {  // Found at beginning
                if ((rxpacket[PKT_ID] > 0xFD) || (rxpacket[PKT_LENGTH] > RXPACKET_MAX_LEN) ||
                    (rxpacket[PKT_ERROR] > 0x7F)) {
                    // Invalid packet, remove first byte
                    rxpacket.erase(rxpacket.begin());
                    rx_length--;
                    continue;
                }

                // Re-calculate exact length
                if (wait_length != static_cast<size_t>(rxpacket[PKT_LENGTH] + PKT_LENGTH + 1)) {
                    wait_length = rxpacket[PKT_LENGTH] + PKT_LENGTH + 1;
                    continue;
                }

                if (rx_length < wait_length) {
                    // Check timeout
                    if (port_handler_->isPacketTimeout()) {
                        result = (rx_length == 0) ? COMM_RX_TIMEOUT : COMM_RX_CORRUPT;
                        break;
                    }
                    continue;
                }

                // Calculate checksum
                for (size_t i = 2; i < wait_length - 1; ++i) {
                    checksum += rxpacket[i];
                }
                checksum = ~checksum & 0xFF;

                // Verify checksum
                if (rxpacket[wait_length - 1] == checksum) {
                    result = COMM_SUCCESS;
                } else {
                    result = COMM_RX_CORRUPT;
                }
                break;

            } else {
                // Remove unnecessary packets
                rxpacket.erase(rxpacket.begin(), rxpacket.begin() + idx);
                rx_length -= idx;
            }
        } else {
            // Check timeout
            if (port_handler_->isPacketTimeout()) {
                result = (rx_length == 0) ? COMM_RX_TIMEOUT : COMM_RX_CORRUPT;
                break;
            }
        }
    }

    port_handler_->setUsing(false);
    return std::make_tuple(rxpacket, result);
}

std::tuple<std::vector<uint8_t>, int, uint8_t> ProtocolPacketHandler::txRxPacket(std::vector<uint8_t>& txpacket) {
    std::vector<uint8_t> rxpacket;
    uint8_t error = 0;

    // Transmit packet
    int result = txPacket(txpacket);
    if (result != COMM_SUCCESS) {
        return std::make_tuple(rxpacket, result, error);
    }

    // If broadcast, no need to wait for response
    if (txpacket[PKT_ID] == BROADCAST_ID) {
        port_handler_->setUsing(false);
        return std::make_tuple(rxpacket, result, error);
    }

    // Set packet timeout
    if (txpacket[PKT_INSTRUCTION] == INST_READ) {
        port_handler_->setPacketTimeout(txpacket[PKT_PARAMETER0 + 1] + 6);
    } else {
        port_handler_->setPacketTimeout(6);
    }

    // Receive packet
    while (true) {
        std::tie(rxpacket, result) = rxPacket();
        if (result != COMM_SUCCESS || (rxpacket.size() > PKT_ID && txpacket[PKT_ID] == rxpacket[PKT_ID])) {
            break;
        }
    }

    if (result == COMM_SUCCESS && rxpacket.size() > PKT_ERROR && txpacket[PKT_ID] == rxpacket[PKT_ID]) {
        error = rxpacket[PKT_ERROR];
    }

    return std::make_tuple(rxpacket, result, error);
}

std::tuple<uint16_t, int, uint8_t> ProtocolPacketHandler::ping(uint8_t sts_id) {
    uint16_t model_number = 0;
    uint8_t error = 0;

    std::vector<uint8_t> txpacket(6, 0);

    if (sts_id >= BROADCAST_ID) {
        return std::make_tuple(model_number, COMM_NOT_AVAILABLE, error);
    }

    txpacket[PKT_ID] = sts_id;
    txpacket[PKT_LENGTH] = 2;
    txpacket[PKT_INSTRUCTION] = INST_PING;

    std::vector<uint8_t> rxpacket;
    int result;
    std::tie(rxpacket, result, error) = txRxPacket(txpacket);

    if (result == COMM_SUCCESS) {
        std::vector<uint8_t> data;
        std::tie(data, result, error) = readTxRx(sts_id, 3, 2);  // Address 3: Model Number
        if (result == COMM_SUCCESS && data.size() >= 2) {
            model_number = makeWord(data[0], data[1]);
        }
    }

    return std::make_tuple(model_number, result, error);
}

int ProtocolPacketHandler::action(uint8_t sts_id) {
    std::vector<uint8_t> txpacket(6, 0);
    txpacket[PKT_ID] = sts_id;
    txpacket[PKT_LENGTH] = 2;
    txpacket[PKT_INSTRUCTION] = INST_ACTION;

    std::vector<uint8_t> rxpacket;
    int result;
    uint8_t error;
    std::tie(rxpacket, result, error) = txRxPacket(txpacket);

    return result;
}

std::tuple<std::vector<uint8_t>, int, uint8_t> ProtocolPacketHandler::readTxRx(uint8_t sts_id, uint8_t address, uint8_t length) {
    std::vector<uint8_t> data;
    std::vector<uint8_t> txpacket(8, 0);

    if (sts_id >= BROADCAST_ID) {
        return std::make_tuple(data, COMM_NOT_AVAILABLE, 0);
    }

    txpacket[PKT_ID] = sts_id;
    txpacket[PKT_LENGTH] = 4;
    txpacket[PKT_INSTRUCTION] = INST_READ;
    txpacket[PKT_PARAMETER0 + 0] = address;
    txpacket[PKT_PARAMETER0 + 1] = length;

    std::vector<uint8_t> rxpacket;
    int result;
    uint8_t error;
    std::tie(rxpacket, result, error) = txRxPacket(txpacket);

    if (result == COMM_SUCCESS && rxpacket.size() >= static_cast<size_t>(PKT_PARAMETER0 + length)) {
        error = rxpacket[PKT_ERROR];
        data.insert(data.end(), rxpacket.begin() + PKT_PARAMETER0, rxpacket.begin() + PKT_PARAMETER0 + length);
    }

    return std::make_tuple(data, result, error);
}

std::tuple<uint8_t, int, uint8_t> ProtocolPacketHandler::read1ByteTxRx(uint8_t sts_id, uint8_t address) {
    std::vector<uint8_t> data;
    int result;
    uint8_t error;
    std::tie(data, result, error) = readTxRx(sts_id, address, 1);
    uint8_t data_read = (result == COMM_SUCCESS && !data.empty()) ? data[0] : 0;
    return std::make_tuple(data_read, result, error);
}

std::tuple<uint16_t, int, uint8_t> ProtocolPacketHandler::read2ByteTxRx(uint8_t sts_id, uint8_t address) {
    std::vector<uint8_t> data;
    int result;
    uint8_t error;
    std::tie(data, result, error) = readTxRx(sts_id, address, 2);
    uint16_t data_read = (result == COMM_SUCCESS && data.size() >= 2) ? makeWord(data[0], data[1]) : 0;
    return std::make_tuple(data_read, result, error);
}

std::tuple<uint32_t, int, uint8_t> ProtocolPacketHandler::read4ByteTxRx(uint8_t sts_id, uint8_t address) {
    std::vector<uint8_t> data;
    int result;
    uint8_t error;
    std::tie(data, result, error) = readTxRx(sts_id, address, 4);
    uint32_t data_read = 0;
    if (result == COMM_SUCCESS && data.size() >= 4) {
        data_read = makeDWord(makeWord(data[0], data[1]), makeWord(data[2], data[3]));
    }
    return std::make_tuple(data_read, result, error);
}

int ProtocolPacketHandler::writeTxOnly(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> txpacket(length + 7, 0);

    txpacket[PKT_ID] = sts_id;
    txpacket[PKT_LENGTH] = length + 3;
    txpacket[PKT_INSTRUCTION] = INST_WRITE;
    txpacket[PKT_PARAMETER0] = address;

    for (uint8_t i = 0; i < length; ++i) {
        txpacket[PKT_PARAMETER0 + 1 + i] = data[i];
    }

    int result = txPacket(txpacket);
    port_handler_->setUsing(false);

    return result;
}

std::tuple<int, uint8_t> ProtocolPacketHandler::writeTxRx(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> txpacket(length + 7, 0);

    txpacket[PKT_ID] = sts_id;
    txpacket[PKT_LENGTH] = length + 3;
    txpacket[PKT_INSTRUCTION] = INST_WRITE;
    txpacket[PKT_PARAMETER0] = address;

    for (uint8_t i = 0; i < length; ++i) {
        txpacket[PKT_PARAMETER0 + 1 + i] = data[i];
    }

    std::vector<uint8_t> rxpacket;
    int result;
    uint8_t error;
    std::tie(rxpacket, result, error) = txRxPacket(txpacket);

    return std::make_tuple(result, error);
}

int ProtocolPacketHandler::write1ByteTxOnly(uint8_t sts_id, uint8_t address, uint8_t data) {
    std::vector<uint8_t> data_write = {data};
    return writeTxOnly(sts_id, address, 1, data_write);
}

std::tuple<int, uint8_t> ProtocolPacketHandler::write1ByteTxRx(uint8_t sts_id, uint8_t address, uint8_t data) {
    std::vector<uint8_t> data_write = {data};
    return writeTxRx(sts_id, address, 1, data_write);
}

int ProtocolPacketHandler::write2ByteTxOnly(uint8_t sts_id, uint8_t address, uint16_t data) {
    std::vector<uint8_t> data_write = {lobyte(data), hibyte(data)};
    return writeTxOnly(sts_id, address, 2, data_write);
}

std::tuple<int, uint8_t> ProtocolPacketHandler::write2ByteTxRx(uint8_t sts_id, uint8_t address, uint16_t data) {
    std::vector<uint8_t> data_write = {lobyte(data), hibyte(data)};
    return writeTxRx(sts_id, address, 2, data_write);
}

}  // namespace st3215
