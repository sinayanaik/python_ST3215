#ifndef ST3215_GROUP_SYNC_READ_H
#define ST3215_GROUP_SYNC_READ_H

#include "protocol_packet_handler.h"
#include "values.h"
#include <map>
#include <vector>
#include <cstdint>
#include <tuple>

namespace st3215 {

/**
 * @brief Handles synchronized read operations from multiple servos
 *
 * This class enables reading the same type of data from multiple servos
 * efficiently using sync read packets.
 */
class GroupSyncRead {
public:
    /**
     * @brief Constructor
     * @param ph Protocol packet handler reference
     * @param start_address Starting register address
     * @param data_length Length of data to read per servo
     */
    GroupSyncRead(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length);

    /**
     * @brief Build the parameter list from stored servo IDs
     */
    void makeParam();

    /**
     * @brief Add a servo to the sync read group
     * @param sts_id Servo ID
     * @return true if added, false if already exists
     */
    bool addParam(uint8_t sts_id);

    /**
     * @brief Remove a servo from the sync read group
     * @param sts_id Servo ID to remove
     */
    void removeParam(uint8_t sts_id);

    /**
     * @brief Clear all servo parameters
     */
    void clearParam();

    /**
     * @brief Transmit the sync read request packet
     * @return Communication result
     */
    int txPacket();

    /**
     * @brief Receive and process sync read response
     * @return Communication result
     */
    int rxPacket();

    /**
     * @brief Combined transmit and receive
     * @return Communication result
     */
    int txRxPacket();

    /**
     * @brief Parse received data for a specific servo
     * @param rxpacket Raw received packet data
     * @param sts_id Servo ID to parse data for
     * @param data_length Expected data length
     * @return Tuple of (data, result)
     */
    std::tuple<std::vector<uint8_t>, int> readRx(const std::vector<uint8_t>& rxpacket, uint8_t sts_id, uint8_t data_length);

    /**
     * @brief Check if data is available for a specific servo
     * @param sts_id Servo ID
     * @param address Register address to check
     * @param data_length Length of data requested
     * @return Tuple of (available, error_byte)
     */
    std::tuple<bool, uint8_t> isAvailable(uint8_t sts_id, uint8_t address, uint8_t data_length);

    /**
     * @brief Get data for a specific servo at a specific address
     * @param sts_id Servo ID
     * @param address Register address
     * @param data_length Length of data (1, 2, or 4 bytes)
     * @return Data value
     */
    uint32_t getData(uint8_t sts_id, uint8_t address, uint8_t data_length);

private:
    ProtocolPacketHandler* ph_;
    uint8_t start_address_;
    uint8_t data_length_;
    bool last_result_;
    bool is_param_changed_;
    std::vector<uint8_t> param_;
    std::map<uint8_t, std::vector<uint8_t>> data_dict_;
};

}  // namespace st3215

#endif  // ST3215_GROUP_SYNC_READ_H
