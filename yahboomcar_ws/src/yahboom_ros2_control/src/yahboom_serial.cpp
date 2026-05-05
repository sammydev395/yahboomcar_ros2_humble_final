// D1 stub. Real POSIX termios setup lands at D2 when chassis goes live.

#include "yahboom_ros2_control/yahboom_serial.hpp"

#include <unistd.h>

namespace yahboom_ros2_control {

YahboomSerial::YahboomSerial() = default;

YahboomSerial::~YahboomSerial() { close(); }

bool YahboomSerial::open(const std::string& /*port*/) {
  return false;  // D2: open(O_RDWR | O_NOCTTY | O_NONBLOCK) + termios cfsetspeed B115200
}

void YahboomSerial::close() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

ssize_t YahboomSerial::write_bytes(const uint8_t* /*buf*/, size_t /*n*/) {
  return -1;  // D2
}

ssize_t YahboomSerial::read_bytes(uint8_t* /*buf*/, size_t /*n*/) {
  return -1;  // D2
}

}  // namespace yahboom_ros2_control
