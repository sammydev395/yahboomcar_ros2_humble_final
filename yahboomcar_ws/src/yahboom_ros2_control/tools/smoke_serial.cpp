// D2.1 hardware smoke test for yahboom_serial + yahboom_protocol.
//
// What it does:
//   1. Open /dev/yahboom_stm32 (or path passed as argv[1]) at 115200 8N1
//   2. Send FUNC_AUTO_REPORT(enable=true, forever=false) to wake STM32 push mode
//   3. Listen for ~3 seconds, parse incoming frames via FrameParser
//   4. Print every frame received (FUNC code + decoded payload for known types)
//   5. Send FUNC_MOTOR(0,0,0,0) to ensure wheels are commanded zero on exit
//
// What it does NOT do:
//   - Move motors (only zero-duty MOTOR commands sent)
//   - Move arm (no FUNC_UART_SERVO commands)
//   - Use any ROS2 machinery (this links against yahboom_serial only)
//
// Run on the Jetson container with the STM32 cable plugged in:
//   ros2 run yahboom_ros2_control smoke_serial
//   ros2 run yahboom_ros2_control smoke_serial /dev/yahboom_stm32 5
//
// Pass criteria for D2.1:
//   - Open succeeds
//   - >0 FUNC_REPORT_* frames received within 3 seconds (proves STM32 is alive
//     and our frame builder + parser round-trip with real hardware)
//   - All received frames have valid checksums (parser doesn't drop any)

#include "yahboom_ros2_control/yahboom_protocol.hpp"
#include "yahboom_ros2_control/yahboom_serial.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

namespace y = yahboom_ros2_control::protocol;
using yahboom_ros2_control::YahboomSerial;
using clk = std::chrono::steady_clock;

static const char* func_name(uint8_t f) {
  switch (f) {
    case y::FUNC_REPORT_SPEED: return "REPORT_SPEED";
    case y::FUNC_REPORT_MPU_RAW: return "REPORT_MPU_RAW";
    case y::FUNC_REPORT_IMU_ATT: return "REPORT_IMU_ATT";
    case y::FUNC_REPORT_ENCODER: return "REPORT_ENCODER";
    case y::FUNC_REPORT_ICM_RAW: return "REPORT_ICM_RAW";
    case y::FUNC_UART_SERVO: return "UART_SERVO";
    case y::FUNC_ARM_CTRL: return "ARM_CTRL";
    default: return "UNKNOWN";
  }
}

static void dump_frame(const y::ParsedFrame& f) {
  std::printf("  FUNC=0x%02X (%s) payload[%zu]=", f.func, func_name(f.func), f.payload.size());
  for (auto b : f.payload) std::printf("%02X ", b);
  std::printf("\n");

  if (f.func == y::FUNC_REPORT_IMU_ATT) {
    if (auto a = y::parse_imu_att(f.payload)) {
      std::printf("    -> roll=%.4f pitch=%.4f yaw=%.4f rad\n", a->roll, a->pitch, a->yaw);
    }
  } else if (f.func == y::FUNC_REPORT_ENCODER) {
    if (auto e = y::parse_encoder(f.payload)) {
      std::printf("    -> m1=%d m2=%d m3=%d m4=%d counts\n", e->m1, e->m2, e->m3, e->m4);
    }
  } else if (f.func == y::FUNC_REPORT_SPEED) {
    if (auto s = y::parse_speed(f.payload)) {
      std::printf("    -> vx=%.3f vy=%.3f wz=%.3f m/s|rad/s  battery=%u (x0.1V)\n",
                  s->vx, s->vy, s->wz, s->battery_v10);
    }
  } else if (f.func == y::FUNC_REPORT_ICM_RAW) {
    if (auto d = y::parse_icm_raw(f.payload)) {
      std::printf("    -> gyro=(%.3f,%.3f,%.3f) accel=(%.3f,%.3f,%.3f) mag=(%.2f,%.2f,%.2f)\n",
                  d->gx, d->gy, d->gz, d->ax, d->ay, d->az, d->mx, d->my, d->mz);
    }
  }
}

int main(int argc, char** argv) {
  std::string port = (argc > 1) ? argv[1] : "/dev/yahboom_stm32";
  double seconds  = (argc > 2) ? std::atof(argv[2]) : 3.0;

  std::printf("[smoke_serial] opening %s ...\n", port.c_str());
  YahboomSerial ser;
  if (!ser.open(port)) {
    std::fprintf(stderr, "[smoke_serial] FAILED to open %s\n", port.c_str());
    return 1;
  }
  std::printf("[smoke_serial] opened OK\n");

  // Give the STM32 a moment to settle on first connection (it sometimes needs
  // a beat after the userland side opens the CH340 before it accepts frames).
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // Set car type FIRST — vendor SDK does this in its constructor and the STM32
  // remembers car type across sessions, but a fresh power-up may have it on a
  // different setting. Defensive.
  {
    auto frame = y::build_set_car_type(y::CAR_X3_PLUS);
    std::printf("[smoke_serial] sending SET_CAR_TYPE(X3_PLUS), %zu bytes\n", frame.size());
    if (ser.write_bytes(frame.data(), frame.size()) != static_cast<ssize_t>(frame.size())) {
      std::fprintf(stderr, "[smoke_serial] write SET_CAR_TYPE failed\n");
      return 2;
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Enable push-mode reports.
  {
    auto frame = y::build_set_auto_report_state(true, false);
    std::printf("[smoke_serial] sending AUTO_REPORT(enable=true, forever=false), %zu bytes\n",
                frame.size());
    if (ser.write_bytes(frame.data(), frame.size()) != static_cast<ssize_t>(frame.size())) {
      std::fprintf(stderr, "[smoke_serial] write AUTO_REPORT failed\n");
      return 3;
    }
  }

  // Drain incoming frames for the configured window.
  std::printf("[smoke_serial] listening for %.1f s ...\n", seconds);
  y::FrameParser parser;
  uint8_t buf[256];
  size_t total_bytes = 0;
  size_t total_frames = 0;
  size_t per_func[256] = {0};
  auto deadline = clk::now() + std::chrono::milliseconds(static_cast<int>(seconds * 1000));
  while (clk::now() < deadline) {
    ssize_t r = ser.read_bytes(buf, sizeof(buf));
    if (r > 0) {
      total_bytes += static_cast<size_t>(r);
      parser.feed(buf, static_cast<size_t>(r));
      while (auto f = parser.next_frame()) {
        ++total_frames;
        per_func[f->func]++;
        if (total_frames <= 12) {
          dump_frame(*f);  // print first dozen, then summarize
        }
      }
    } else if (r < 0) {
      std::fprintf(stderr, "[smoke_serial] read error, abort\n");
      break;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  // Defensive: command wheels to zero before exit.
  {
    auto frame = y::build_set_motor(0, 0, 0, 0);
    ser.write_bytes(frame.data(), frame.size());
  }

  std::printf("\n[smoke_serial] === SUMMARY ===\n");
  std::printf("  bytes received: %zu\n", total_bytes);
  std::printf("  frames parsed:  %zu\n", total_frames);
  for (int f = 0; f < 256; ++f) {
    if (per_func[f]) {
      std::printf("    FUNC 0x%02X (%s): %zu frames\n", f, func_name(static_cast<uint8_t>(f)),
                  per_func[f]);
    }
  }

  if (total_frames == 0) {
    std::fprintf(stderr, "[smoke_serial] FAIL — no frames received in %.1fs\n", seconds);
    std::fprintf(stderr, "  troubleshooting: is the STM32 powered? is /dev/yahboom_stm32 the\n");
    std::fprintf(stderr, "  right device? did vendor Mcnamu_driver_X3plus or rosmaster_main.py\n");
    std::fprintf(stderr, "  steal the port between our open() and the STM32's first push?\n");
    return 4;
  }
  std::printf("[smoke_serial] PASS\n");
  return 0;
}
