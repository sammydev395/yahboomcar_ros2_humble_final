// POSIX serial wrapper for the Yahboom STM32 link. Default port
// `/dev/yahboom_stm32` (set up by `provision/jetson/99-yahboom-stm32.rules`,
// pending). 115200 8N1, no flow control. D1: header-only declarations;
// implementation is a stub until D2.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace yahboom_ros2_control {

class YahboomSerial {
 public:
  YahboomSerial();
  ~YahboomSerial();

  YahboomSerial(const YahboomSerial&) = delete;
  YahboomSerial& operator=(const YahboomSerial&) = delete;

  // Open the port at 115200 8N1. Returns false on failure; check errno.
  bool open(const std::string& port);

  void close();

  bool is_open() const { return fd_ >= 0; }

  // Blocking write of n bytes. Returns bytes written or -1.
  ssize_t write_bytes(const uint8_t* buf, size_t n);

  // Non-blocking read up to n bytes; returns immediately. 0 = no data, -1 =
  // error (other than EAGAIN, which returns 0).
  ssize_t read_bytes(uint8_t* buf, size_t n);

 private:
  int fd_ = -1;
};

}  // namespace yahboom_ros2_control
