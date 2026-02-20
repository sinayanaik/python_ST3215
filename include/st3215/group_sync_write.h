#ifndef ST3215_GROUP_SYNC_WRITE_H
#define ST3215_GROUP_SYNC_WRITE_H

#include "protocol_packet_handler.h"
#include "values.h"
#include <map>
#include <vector>
#include <cstdint>

namespace st3215 {

/**
 * @brief Handles synchronized write operations to multiple servos
 *
 * This class enables writing the same type of data to multiple servos
 * in a single packet, improving communication efficiency.
 */
class GroupSyncWrite {
public:
    /**
     * @brief Constructor
     * @param ph Protocol packet handler reference
     * @param start_address Starting register address
     * @param data_length Length of data per servo
     */
    GroupSyncWrite(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length);

    /**
     * @brief Build the parameter list from stored data
     */
    void makeParam();

    /**
     * @brief Add a servo and its data to the sync write group
     * @param sts_id Servo ID
     * @param data Data to write
     * @return true if added successfully, false if ID already exists or data too long
     */
    bool addParam(uint8_t sts_id, const std::vector<uint8_t>& data);

    /**
     * @brief Remove a servo from the sync write group
     * @param sts_id Servo ID to remove
     */
    void removeParam(uint8_t sts_id);

    /**
     * @brief Change data for an existing servo
     * @param sts_id Servo ID
     * @param data New data to write
     * @return true if changed successfully, false if ID doesn't exist or data too long
     */
    bool changeParam(uint8_t sts_id, const std::vector<uint8_t>& data);

    /**
     * @brief Clear all servo parameters
     */
    void clearParam();

    /**
     * @brief Transmit the sync write packet
     * @return Communication result
     */
    int txPacket();

private:
    ProtocolPacketHandler* ph_;
    uint8_t start_address_;
    uint8_t data_length_;
    bool is_param_changed_;
    std::vector<uint8_t> param_;
    std::map<uint8_t, std::vector<uint8_t>> data_dict_;
};

}  // namespace st3215

#endif  // ST3215_GROUP_SYNC_WRITE_H
