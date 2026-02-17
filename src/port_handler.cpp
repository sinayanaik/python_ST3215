#include "st3215/port_handler.h"
#include "st3215/values.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stdexcept>

namespace st3215 {

PortHandler::PortHandler(const std::string& port_name)
    : is_open_(false),
      baudrate_(DEFAULT_BAUDRATE),
      packet_start_time_(0.0),
      packet_timeout_(0.0),
      tx_time_per_byte_(0.0),
      is_using_(false),
      port_name_(port_name),
      serial_fd_(-1) {
}

PortHandler::~PortHandler() {
    if (is_open_) {
        closePort();
    }
}

bool PortHandler::openPort() {
    return setupPort();
}

void PortHandler::closePort() {
    if (serial_fd_ != -1) {
        close(serial_fd_);
        serial_fd_ = -1;
    }
    is_open_ = false;
}

void PortHandler::clearPort() {
    if (serial_fd_ != -1) {
        tcflush(serial_fd_, TCIOFLUSH);
    }
}

void PortHandler::setPortName(const std::string& port_name) {
    port_name_ = port_name;
}

std::string PortHandler::getPortName() const {
    return port_name_;
}

uint32_t PortHandler::getBaudRate() const {
    return baudrate_;
}

bool PortHandler::setBaudRate(uint32_t baudrate) {
    baudrate_ = baudrate;
    if (is_open_) {
        closePort();
        return setupPort();
    }
    return true;
}

size_t PortHandler::getBytesAvailable() {
    int bytes_available = 0;
    if (serial_fd_ != -1) {
        ioctl(serial_fd_, FIONREAD, &bytes_available);
    }
    return static_cast<size_t>(bytes_available);
}

std::vector<uint8_t> PortHandler::readPort(size_t length) {
    std::vector<uint8_t> buffer(length);
    if (serial_fd_ != -1 && length > 0) {
        ssize_t bytes_read = read(serial_fd_, buffer.data(), length);
        if (bytes_read > 0) {
            buffer.resize(static_cast<size_t>(bytes_read));
        } else {
            buffer.clear();
        }
    } else {
        buffer.clear();
    }
    return buffer;
}

size_t PortHandler::writePort(const std::vector<uint8_t>& packet) {
    if (serial_fd_ == -1 || packet.empty()) {
        return 0;
    }
    
    ssize_t bytes_written = write(serial_fd_, packet.data(), packet.size());
    if (bytes_written < 0) {
        return 0;
    }
    
    return static_cast<size_t>(bytes_written);
}

void PortHandler::setPacketTimeout(size_t packet_length) {
    packet_start_time_ = getCurrentTime();
    packet_timeout_ = (tx_time_per_byte_ * packet_length) + (tx_time_per_byte_ * 3.0) + LATENCY_TIMER;
}

void PortHandler::setPacketTimeoutMillis(double msec) {
    packet_start_time_ = getCurrentTime();
    packet_timeout_ = msec;
}

bool PortHandler::isPacketTimeout() {
    if (getTimeSinceStart() > packet_timeout_) {
        packet_timeout_ = 0;
        return true;
    }
    return false;
}

double PortHandler::getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double, std::milli>(duration).count();
}

double PortHandler::getTimeSinceStart() {
    double time_since = getCurrentTime() - packet_start_time_;
    if (time_since < 0.0) {
        packet_start_time_ = getCurrentTime();
        return 0.0;
    }
    return time_since;
}

bool PortHandler::setupPort() {
    if (is_open_) {
        closePort();
    }

    // Open serial port
    serial_fd_ = open(port_name_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd_ == -1) {
        return false;
    }

    // Configure port
    struct termios options;
    if (tcgetattr(serial_fd_, &options) != 0) {
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    // Set baudrate
    speed_t baud_constant;
    switch (baudrate_) {
        case 1000000: baud_constant = B1000000; break;
        case 500000: baud_constant = B500000; break;
        case 115200: baud_constant = B115200; break;
        case 57600: baud_constant = B57600; break;
        case 38400: baud_constant = B38400; break;
        default: baud_constant = B1000000; break;
    }

    cfsetispeed(&options, baud_constant);
    cfsetospeed(&options, baud_constant);

    // 8N1 mode
    options.c_cflag &= ~PARENB;  // No parity
    options.c_cflag &= ~CSTOPB;  // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;      // 8 data bits
    options.c_cflag |= (CLOCAL | CREAD);  // Enable receiver, ignore modem control lines

    // Raw input mode
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);  // Disable software flow control
    options.c_oflag &= ~OPOST;  // Raw output

    // Set read timeout to 0 (non-blocking)
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    // Apply settings
    if (tcsetattr(serial_fd_, TCSANOW, &options) != 0) {
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    // Flush buffers
    tcflush(serial_fd_, TCIOFLUSH);

    is_open_ = true;

    // Calculate transmission time per byte
    tx_time_per_byte_ = (1000.0 / baudrate_) * 10.0;

    return true;
}

}  // namespace st3215
