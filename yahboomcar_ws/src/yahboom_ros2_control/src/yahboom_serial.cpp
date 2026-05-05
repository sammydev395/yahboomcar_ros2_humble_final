// POSIX termios serial wrapper for the Yahboom STM32 link.
// 115200 8N1, no flow control, raw mode, non-blocking I/O.

#include "yahboom_ros2_control/yahboom_serial.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace yahboom_ros2_control {

YahboomSerial::YahboomSerial() = default;

YahboomSerial::~YahboomSerial() { close(); }

bool YahboomSerial::open(const std::string& port) {
  if (fd_ >= 0) close();

  fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    std::fprintf(stderr, "[YahboomSerial] open(%s) failed: %s\n",
                 port.c_str(), std::strerror(errno));
    return false;
  }

  struct termios tio{};
  if (tcgetattr(fd_, &tio) != 0) {
    std::fprintf(stderr, "[YahboomSerial] tcgetattr failed: %s\n", std::strerror(errno));
    close();
    return false;
  }

  // Raw mode: clears canonical processing, signal generation, echo, etc.
  cfmakeraw(&tio);

  // 115200 baud both directions.
  if (cfsetispeed(&tio, B115200) != 0 || cfsetospeed(&tio, B115200) != 0) {
    std::fprintf(stderr, "[YahboomSerial] cfsetspeed(B115200) failed: %s\n",
                 std::strerror(errno));
    close();
    return false;
  }

  // 8N1, no flow control, ignore modem status, allow read.
  tio.c_cflag |= (CLOCAL | CREAD);
  tio.c_cflag &= ~CRTSCTS;       // no hardware flow control
  tio.c_cflag &= ~PARENB;        // no parity
  tio.c_cflag &= ~CSTOPB;        // 1 stop bit
  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |=  CS8;           // 8 data bits

  tio.c_iflag &= ~(IXON | IXOFF | IXANY);  // no software flow control
  tio.c_iflag &= ~(INLCR | ICRNL | IGNCR); // no CR/LF translation

  // Non-blocking read: return immediately with whatever is available (or 0).
  // We rely on read_bytes() callers handling 0-byte returns as "no data yet".
  tio.c_cc[VMIN]  = 0;
  tio.c_cc[VTIME] = 0;

  if (tcsetattr(fd_, TCSANOW, &tio) != 0) {
    std::fprintf(stderr, "[YahboomSerial] tcsetattr failed: %s\n", std::strerror(errno));
    close();
    return false;
  }

  // Drop any stale bytes in kernel buffers from a previous owner of the port.
  tcflush(fd_, TCIOFLUSH);

  return true;
}

void YahboomSerial::close() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

ssize_t YahboomSerial::write_bytes(const uint8_t* buf, size_t n) {
  if (fd_ < 0) return -1;
  // Loop on partial writes — small frames (<20 bytes) almost always go in one
  // shot, but for safety we loop until n bytes are out or a real error.
  size_t total = 0;
  while (total < n) {
    ssize_t r = ::write(fd_, buf + total, n - total);
    if (r > 0) {
      total += static_cast<size_t>(r);
    } else if (r < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Output buffer full; in practice this shouldn't happen at our rates,
        // but if it does we'd want to back off. For now fail loud.
        std::fprintf(stderr, "[YahboomSerial] write_bytes EAGAIN at %zu/%zu\n", total, n);
        return -1;
      }
      if (errno == EINTR) continue;
      std::fprintf(stderr, "[YahboomSerial] write_bytes error: %s\n", std::strerror(errno));
      return -1;
    } else {
      // r == 0 — shouldn't happen on a serial port; treat as transient.
      break;
    }
  }
  return static_cast<ssize_t>(total);
}

ssize_t YahboomSerial::read_bytes(uint8_t* buf, size_t n) {
  if (fd_ < 0) return -1;
  ssize_t r = ::read(fd_, buf, n);
  if (r >= 0) return r;
  if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;  // no data available
  if (errno == EINTR) return 0;                            // try again next tick
  std::fprintf(stderr, "[YahboomSerial] read_bytes error: %s\n", std::strerror(errno));
  return -1;
}

}  // namespace yahboom_ros2_control
