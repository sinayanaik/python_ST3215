#ifndef ST3215_PROTOCOL_PACKET_HANDLER_H
#define ST3215_PROTOCOL_PACKET_HANDLER_H

#include "port_handler.h"
#include "values.h"
#include <vector>
#include <string>
#include <cstdint>
#include <tuple>

namespace st3215 {

/**
 * @brief Handles protocol-level packet communication
 * 
 * This class implements the STS protocol for packet-based communication
 * with ST3215 servo motors.
 */
class ProtocolPacketHandler {
public:
    /**
     * @brief Constructor
     * @param port_handler Pointer to the port handler
     */
    explicit ProtocolPacketHandler(PortHandler* port_handler);

    /**
     * @brief Get protocol version
     * @return Protocol version (1.0)
     */
    float getProtocolVersion() const { return 1.0f; }

    /**
     * @brief Get human-readable result message
     * @param result Communication result code
     * @return Result message string
     */
    std::string getTxRxResult(int result) const;

    /**
     * @brief Get human-readable error message
     * @param error Error code
     * @return Error message string
     */
    std::string getRxPacketError(uint8_t error) const;

    /**
     * @brief Ping a servo
     * @param sts_id Servo ID
     * @return Tuple of (model_number, comm_result, error)
     */
    std::tuple<uint16_t, int, uint8_t> ping(uint8_t sts_id);

    /**
     * @brief Send action command
     * @param sts_id Servo ID
     * @return Communication result
     */
    int action(uint8_t sts_id);

    /**
     * @brief Read data from servo (transaction)
     * @param sts_id Servo ID
     * @param address Register address
     * @param length Number of bytes to read
     * @return Tuple of (data, comm_result, error)
     */
    std::tuple<std::vector<uint8_t>, int, uint8_t> readTxRx(uint8_t sts_id, uint8_t address, uint8_t length);

    /**
     * @brief Read 1 byte from servo
     * @param sts_id Servo ID
     * @param address Register address
     * @return Tuple of (data, comm_result, error)
     */
    std::tuple<uint8_t, int, uint8_t> read1ByteTxRx(uint8_t sts_id, uint8_t address);

    /**
     * @brief Read 2 bytes from servo
     * @param sts_id Servo ID
     * @param address Register address
     * @return Tuple of (data, comm_result, error)
     */
    std::tuple<uint16_t, int, uint8_t> read2ByteTxRx(uint8_t sts_id, uint8_t address);

    /**
     * @brief Read 4 bytes from servo
     * @param sts_id Servo ID
     * @param address Register address
     * @return Tuple of (data, comm_result, error)
     */
    std::tuple<uint32_t, int, uint8_t> read4ByteTxRx(uint8_t sts_id, uint8_t address);

    /**
     * @brief Write data to servo (no response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param length Number of bytes to write
     * @param data Data to write
     * @return Communication result
     */
    int writeTxOnly(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);

    /**
     * @brief Write data to servo (with response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param length Number of bytes to write
     * @param data Data to write
     * @return Tuple of (comm_result, error)
     */
    std::tuple<int, uint8_t> writeTxRx(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);

    /**
     * @brief Write 1 byte to servo (no response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param data Data to write
     * @return Communication result
     */
    int write1ByteTxOnly(uint8_t sts_id, uint8_t address, uint8_t data);

    /**
     * @brief Write 1 byte to servo (with response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param data Data to write
     * @return Tuple of (comm_result, error)
     */
    std::tuple<int, uint8_t> write1ByteTxRx(uint8_t sts_id, uint8_t address, uint8_t data);

    /**
     * @brief Write 2 bytes to servo (no response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param data Data to write
     * @return Communication result
     */
    int write2ByteTxOnly(uint8_t sts_id, uint8_t address, uint16_t data);

    /**
     * @brief Write 2 bytes to servo (with response)
     * @param sts_id Servo ID
     * @param address Register address
     * @param data Data to write
     * @return Tuple of (comm_result, error)
     */
    std::tuple<int, uint8_t> write2ByteTxRx(uint8_t sts_id, uint8_t address, uint16_t data);

    // Split read/write Tx and Rx methods
    int readTx(uint8_t sts_id, uint8_t address, uint8_t length);
    std::tuple<std::vector<uint8_t>, int, uint8_t> readRx(uint8_t sts_id, uint8_t length);

    int read1ByteTx(uint8_t sts_id, uint8_t address);
    std::tuple<uint8_t, int, uint8_t> read1ByteRx(uint8_t sts_id);
    int read2ByteTx(uint8_t sts_id, uint8_t address);
    std::tuple<uint16_t, int, uint8_t> read2ByteRx(uint8_t sts_id);
    int read4ByteTx(uint8_t sts_id, uint8_t address);
    std::tuple<uint32_t, int, uint8_t> read4ByteRx(uint8_t sts_id);

    int write4ByteTxOnly(uint8_t sts_id, uint8_t address, uint32_t data);
    std::tuple<int, uint8_t> write4ByteTxRx(uint8_t sts_id, uint8_t address, uint32_t data);

    int regWriteTxOnly(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);
    std::tuple<int, uint8_t> regWriteTxRx(uint8_t sts_id, uint8_t address, uint8_t length, const std::vector<uint8_t>& data);

    int syncReadTx(uint8_t start_address, uint8_t data_length, const std::vector<uint8_t>& param, size_t param_length);
    std::tuple<int, std::vector<uint8_t>> syncReadRx(uint8_t data_length, size_t param_length);
    int syncWriteTxOnly(uint8_t start_address, uint8_t data_length, const std::vector<uint8_t>& param, size_t param_length);

    // Helper functions for byte manipulation
    uint16_t makeWord(uint8_t a, uint8_t b) const;
    uint32_t makeDWord(uint16_t a, uint16_t b) const;
    uint8_t lobyte(uint16_t w) const;
    uint8_t hibyte(uint16_t w) const;
    int16_t toHost(uint16_t a, uint8_t b) const;
    int16_t toScs(int16_t a, uint8_t b) const;
    uint16_t loword(uint32_t l) const;
    uint16_t hiword(uint32_t h) const;
    void setEnd(uint8_t end) { sts_end_ = end; }
    uint8_t getEnd() const { return sts_end_; }

protected:
    /**
     * @brief Transmit a packet
     * @param txpacket Packet to transmit
     * @return Communication result
     */
    int txPacket(std::vector<uint8_t>& txpacket);

    /**
     * @brief Receive a packet
     * @return Tuple of (rxpacket, result)
     */
    std::tuple<std::vector<uint8_t>, int> rxPacket();

    /**
     * @brief Transmit and receive packets
     * @param txpacket Packet to transmit
     * @return Tuple of (rxpacket, result, error)
     */
    std::tuple<std::vector<uint8_t>, int, uint8_t> txRxPacket(std::vector<uint8_t>& txpacket);

    PortHandler* port_handler_;
    uint8_t sts_end_;  // Endianness (0 for little-endian)
};

}  // namespace st3215

#endif  // ST3215_PROTOCOL_PACKET_HANDLER_H
