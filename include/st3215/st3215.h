#ifndef ST3215_H
#define ST3215_H

#include "protocol_packet_handler.h"
#include "group_sync_write.h"
#include "port_handler.h"
#include "values.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <optional>

namespace st3215 {

/**
 * @brief Main class for controlling ST3215 servo motors
 * 
 * This class provides a high-level API for controlling ST3215 servo motors,
 * including motion control, configuration, and diagnostics.
 */
class ST3215 : public ProtocolPacketHandler {
public:
    /**
     * @brief Constructor
     * @param device Serial port device name (e.g., "/dev/ttyUSB0")
     * @throws std::runtime_error if port cannot be opened
     */
    explicit ST3215(const std::string& device);

    /**
     * @brief Destructor
     */
    ~ST3215();

    /// Synchronized write handler for multi-servo writes
    std::unique_ptr<GroupSyncWrite> groupSyncWrite;

    // Servo Discovery and Communication
    
    /**
     * @brief Check if a servo is present and responding
     * @param sts_id Servo ID
     * @return true if servo responds, false otherwise
     */
    bool pingServo(uint8_t sts_id);

    /**
     * @brief Scan bus and list all connected servos
     * @return Vector of servo IDs
     */
    std::vector<uint8_t> listServos();

    // Read Operations

    /**
     * @brief Read servo load in percentage
     * @param sts_id Servo ID
     * @return Load value (0-100%), or nullopt on error
     */
    std::optional<double> readLoad(uint8_t sts_id);

    /**
     * @brief Read servo voltage in volts
     * @param sts_id Servo ID
     * @return Voltage value, or nullopt on error
     */
    std::optional<double> readVoltage(uint8_t sts_id);

    /**
     * @brief Read servo current in milliamps
     * @param sts_id Servo ID
     * @return Current value in mA, or nullopt on error
     */
    std::optional<double> readCurrent(uint8_t sts_id);

    /**
     * @brief Read servo temperature in Celsius
     * @param sts_id Servo ID
     * @return Temperature value, or nullopt on error
     */
    std::optional<int> readTemperature(uint8_t sts_id);

    /**
     * @brief Read servo acceleration setting
     * @param sts_id Servo ID
     * @return Acceleration value, or nullopt on error
     */
    std::optional<uint8_t> readAcceleration(uint8_t sts_id);

    /**
     * @brief Read servo mode
     * @param sts_id Servo ID
     * @return Mode (0=Position, 1=Speed, 2=PWM, 3=Step), or nullopt on error
     */
    std::optional<uint8_t> readMode(uint8_t sts_id);

    /**
     * @brief Read position correction value
     * @param sts_id Servo ID
     * @return Correction value, or nullopt on error
     */
    std::optional<int16_t> readCorrection(uint8_t sts_id);

    /**
     * @brief Check if servo is moving
     * @param sts_id Servo ID
     * @return true if moving, false if stopped, nullopt on error
     */
    std::optional<bool> isMoving(uint8_t sts_id);

    /**
     * @brief Read current position
     * @param sts_id Servo ID
     * @return Position value (0-4095), or nullopt on error
     */
    std::optional<uint16_t> readPosition(uint8_t sts_id);

    /**
     * @brief Read current speed
     * @param sts_id Servo ID
     * @return Tuple of (speed, comm_result, error)
     */
    std::tuple<int16_t, int, uint8_t> readSpeed(uint8_t sts_id);

    /**
     * @brief Read servo status
     * @param sts_id Servo ID
     * @return Map of status bits, or nullopt on error
     */
    std::optional<std::map<std::string, bool>> readStatus(uint8_t sts_id);

    // Write Operations

    /**
     * @brief Set acceleration
     * @param sts_id Servo ID
     * @param acc Acceleration value (0-254, unit: 100 step/s²)
     * @return true on success, false on error
     */
    bool setAcceleration(uint8_t sts_id, uint8_t acc);

    /**
     * @brief Set speed
     * @param sts_id Servo ID
     * @param speed Speed value (0-3400, unit: step/s)
     * @return true on success, false on error
     */
    bool setSpeed(uint8_t sts_id, uint16_t speed);

    /**
     * @brief Stop servo (disable torque)
     * @param sts_id Servo ID
     * @return true on success, false on error
     */
    bool stopServo(uint8_t sts_id);

    /**
     * @brief Start servo (enable torque)
     * @param sts_id Servo ID
     * @return true on success, false on error
     */
    bool startServo(uint8_t sts_id);

    /**
     * @brief Set operational mode
     * @param sts_id Servo ID
     * @param mode Mode (0=Position, 1=Speed, 2=PWM, 3=Step)
     * @return true on success, false on error
     */
    bool setMode(uint8_t sts_id, uint8_t mode);

    /**
     * @brief Set position correction
     * @param sts_id Servo ID
     * @param correction Correction value (can be negative)
     * @return true on success, false on error
     */
    bool correctPosition(uint8_t sts_id, int16_t correction);

    /**
     * @brief Start continuous rotation
     * @param sts_id Servo ID
     * @param speed Rotation speed (negative for counterclockwise)
     * @return true on success, false on error
     */
    bool rotate(uint8_t sts_id, int16_t speed);

    /**
     * @brief Move servo to target position
     * @param sts_id Servo ID
     * @param position Target position (0-4095)
     * @param speed Movement speed (default: 2400 step/s)
     * @param acc Acceleration (default: 50, unit: 100 step/s²)
     * @param wait Wait for movement to complete (default: false)
     * @return true on success, false on error
     */
    bool moveTo(uint8_t sts_id, uint16_t position, uint16_t speed = 2400, uint8_t acc = 50, bool wait = false);

    /**
     * @brief Write position (low-level)
     * @param sts_id Servo ID
     * @param position Target position
     * @return true on success, false on error
     */
    bool writePosition(uint8_t sts_id, uint16_t position);

    // EEPROM Operations

    /**
     * @brief Lock EEPROM
     * @param sts_id Servo ID
     * @return Communication result
     */
    int lockEprom(uint8_t sts_id);

    /**
     * @brief Unlock EEPROM
     * @param sts_id Servo ID
     * @return Communication result
     */
    int unlockEprom(uint8_t sts_id);

    /**
     * @brief Change servo ID
     * @param sts_id Current servo ID
     * @param new_id New servo ID (0-253)
     * @return Empty string on success, error message on failure
     */
    std::string changeId(uint8_t sts_id, uint8_t new_id);

    /**
     * @brief Change servo baudrate
     * @param sts_id Servo ID
     * @param new_baudrate New baudrate code (0-7)
     * @return Empty string on success, error message on failure
     */
    std::string changeBaudrate(uint8_t sts_id, uint8_t new_baudrate);

    // Advanced Operations

    /**
     * @brief Define middle position (set torque to 128)
     * @param sts_id Servo ID
     * @return true on success, false on error
     */
    bool defineMiddle(uint8_t sts_id);

    /**
     * @brief Calibrate servo (find min/max positions)
     * @param sts_id Servo ID
     * @return Tuple of (min_position, max_position), both nullopt on error
     */
    std::tuple<std::optional<uint16_t>, std::optional<uint16_t>> tareServo(uint8_t sts_id);

private:
    /**
     * @brief Get next blocking position (internal helper)
     * @param sts_id Servo ID
     * @return Position or nullopt on error
     */
    std::optional<uint16_t> getBlockPosition(uint8_t sts_id);

    std::unique_ptr<PortHandler> port_handler_;
    std::mutex lock_;
};

}  // namespace st3215

#endif  // ST3215_H
