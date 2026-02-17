#ifndef ST3215_PORT_HANDLER_H
#define ST3215_PORT_HANDLER_H

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace st3215 {

/**
 * @brief Handles serial port communication for ST3215 servos
 * 
 * This class provides low-level serial communication with ST3215 servo motors.
 * It manages the serial port, timing, and buffering.
 */
class PortHandler {
public:
    /**
     * @brief Constructor
     * @param port_name Serial port device name (e.g., "/dev/ttyUSB0" on Linux)
     */
    explicit PortHandler(const std::string& port_name);
    
    /**
     * @brief Destructor - closes the port if open
     */
    ~PortHandler();

    // Disable copy and move to prevent port conflicts
    PortHandler(const PortHandler&) = delete;
    PortHandler& operator=(const PortHandler&) = delete;
    PortHandler(PortHandler&&) = delete;
    PortHandler& operator=(PortHandler&&) = delete;

    /**
     * @brief Open the serial port
     * @return true if successful, false otherwise
     */
    bool openPort();

    /**
     * @brief Close the serial port
     */
    void closePort();

    /**
     * @brief Clear the port buffers
     */
    void clearPort();

    /**
     * @brief Set the port name
     * @param port_name New port name
     */
    void setPortName(const std::string& port_name);

    /**
     * @brief Get the port name
     * @return Current port name
     */
    std::string getPortName() const;

    /**
     * @brief Get the current baudrate
     * @return Baudrate value
     */
    uint32_t getBaudRate() const;

    /**
     * @brief Set the baudrate
     * @param baudrate New baudrate value
     * @return true if successful, false otherwise
     */
    bool setBaudRate(uint32_t baudrate);

    /**
     * @brief Get number of bytes available to read
     * @return Number of available bytes
     */
    size_t getBytesAvailable();

    /**
     * @brief Read data from the port
     * @param length Number of bytes to read
     * @return Vector containing read bytes
     */
    std::vector<uint8_t> readPort(size_t length);

    /**
     * @brief Write data to the port
     * @param packet Data to write
     * @return Number of bytes written
     */
    size_t writePort(const std::vector<uint8_t>& packet);

    /**
     * @brief Set timeout for packet reception
     * @param packet_length Expected packet length
     */
    void setPacketTimeout(size_t packet_length);

    /**
     * @brief Set timeout in milliseconds
     * @param msec Timeout value in milliseconds
     */
    void setPacketTimeoutMillis(double msec);

    /**
     * @brief Check if packet timeout has occurred
     * @return true if timeout occurred, false otherwise
     */
    bool isPacketTimeout();

    /**
     * @brief Get current time in milliseconds
     * @return Current time
     */
    double getCurrentTime();

    /**
     * @brief Get time elapsed since packet start
     * @return Elapsed time in milliseconds
     */
    double getTimeSinceStart();

    /**
     * @brief Check if port is open
     * @return true if port is open, false otherwise
     */
    bool isOpen() const { return is_open_; }

    /**
     * @brief Check if port is currently in use
     * @return true if port is in use, false otherwise
     */
    bool isUsing() const { return is_using_; }

    /**
     * @brief Set port usage flag
     * @param using_flag New usage flag value
     */
    void setUsing(bool using_flag) { is_using_ = using_flag; }

private:
    bool setupPort();

    bool is_open_;
    uint32_t baudrate_;
    double packet_start_time_;
    double packet_timeout_;
    double tx_time_per_byte_;
    bool is_using_;
    std::string port_name_;
    int serial_fd_;  // File descriptor for serial port
};

}  // namespace st3215

#endif  // ST3215_PORT_HANDLER_H
