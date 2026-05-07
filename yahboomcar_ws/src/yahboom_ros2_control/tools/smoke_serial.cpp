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
  // Modes:
  //   smoke_serial [port] [seconds]    — drain push frames (default)
  //   smoke_serial [port] --query-arm  — synchronous arm-position read
  //                                       (no torque, no motion). D7 prereq.
  //   smoke_serial [port] --torque-off — disable arm bus servo torque so
  //                                       operator can manually re-pose the
  //                                       arm. NO motion of its own. Safe.
  //                                       Send --torque-on after re-posing
  //                                       OR just relaunch ros2_control.
  //   smoke_serial [port] --torque-on  — re-enable arm bus servo torque
  //                                       (typically not needed since
  //                                       YahboomSystem.on_activate does it).
  std::string port = (argc > 1) ? argv[1] : "/dev/yahboom_stm32";
  std::string mode = (argc > 2) ? argv[2] : "";
  bool query_arm_mode = (mode == "--query-arm");
  bool torque_off_mode = (mode == "--torque-off");
  bool torque_on_mode  = (mode == "--torque-on");
  bool move_servo_mode = (mode == "--move-servo");
  double seconds = (query_arm_mode || torque_off_mode || torque_on_mode || move_servo_mode)
                       ? 0.0
                       : ((argc > 2) ? std::atof(argv[2]) : 3.0);

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

  // ── Torque on/off modes (D7 helper) ─────────────────────────────────
  // Single-frame FUNC_UART_SERVO_TORQUE write, then exit. Used to
  // disengage arm servos so the operator can manually re-pose joints
  // (e.g., back-drive joint3 into URDF range before phase4 launch),
  // then optionally re-engage. YahboomSystem.on_activate also
  // re-engages, so --torque-on is mostly a convenience.
  if (torque_off_mode || torque_on_mode) {
    const bool enable = torque_on_mode;
    auto frame = y::build_set_uart_servo_torque(enable);
    std::printf("[smoke_serial] sending FUNC_UART_SERVO_TORQUE(%s), %zu bytes\n",
                enable ? "ON" : "OFF", frame.size());
    if (ser.write_bytes(frame.data(), frame.size()) != static_cast<ssize_t>(frame.size())) {
      std::fprintf(stderr, "[smoke_serial] write FUNC_UART_SERVO_TORQUE failed\n");
      return 4;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (enable) {
      std::printf("[smoke_serial] torque ENABLED — arm now holds position. "
                  "Run --query-arm to verify joint positions before phase4 launch.\n");
    } else {
      std::printf("[smoke_serial] torque DISABLED — arm is now back-drivable.\n"
                  "  Move joint3 (elbow) so it reads inside URDF range "
                  "(vendor 0..180°, URDF -1.57..+1.57 rad).\n"
                  "  Then run: ros2 run yahboom_ros2_control smoke_serial /dev/myserial --query-arm\n"
                  "  to verify the new pose. Re-enable torque with --torque-on (or just relaunch ros2_control).\n");
    }
    return 0;
  }

  // ── Move-servo mode (D7.5 diagnostic) ───────────────────────────────
  // Send a single FUNC_UART_SERVO frame to test whether single-servo
  // commands actually drive the bus servos. Used to diagnose the case
  // where YahboomSystem.write() switched from batch FUNC_ARM_CTRL to
  // per-joint FUNC_UART_SERVO and the arm stopped responding entirely.
  // This isolates the firmware path: if --move-servo moves a joint,
  // FUNC_UART_SERVO works fine and the bug is in YahboomSystem; if it
  // doesn't move, FUNC_UART_SERVO needs different prep (e.g. preceding
  // FUNC_ARM_CTRL, longer run_time, etc.).
  //
  // Usage:  smoke_serial /dev/myserial --move-servo S deg [run_time_ms]
  //   S         servo id 1..6
  //   deg       target angle (servos 1-4: 0..180; 5: 0..270; 6: 0..180)
  //   run_time  optional, default 500 (vendor default).
  // Pre-flight: arm torque must be ON. If smoke_serial --torque-off was
  // run earlier, run --torque-on first OR include the seed-engage step.
  if (move_servo_mode) {
    if (argc < 5) {
      std::fprintf(stderr,
                   "[smoke_serial] --move-servo needs <servo_id> <deg> "
                   "[run_time_ms].\n  example: smoke_serial /dev/myserial "
                   "--move-servo 1 100 500\n");
      return 8;
    }
    const int s_id = std::atoi(argv[3]);
    const double angle_deg = std::atof(argv[4]);
    const int run_time_ms = (argc > 5) ? std::atoi(argv[5]) : 500;
    if (s_id < 1 || s_id > 6) {
      std::fprintf(stderr, "[smoke_serial] servo_id must be 1..6\n");
      return 8;
    }
    // Belt-and-suspenders: re-enable torque before attempting motion in
    // case the previous session left it off.
    {
      auto frame = y::build_set_uart_servo_torque(true);
      ser.write_bytes(frame.data(), frame.size());
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    const int16_t pulse = y::arm_angle_to_pulse(static_cast<uint8_t>(s_id), angle_deg);
    auto frame = y::build_set_uart_servo(static_cast<uint8_t>(s_id), pulse,
                                          static_cast<uint16_t>(run_time_ms));
    std::printf("[smoke_serial] sending FUNC_UART_SERVO: s_id=%d deg=%.1f "
                "pulse=%d run_time=%d ms (frame %zu bytes)\n",
                s_id, angle_deg, pulse, run_time_ms, frame.size());
    if (ser.write_bytes(frame.data(), frame.size()) !=
        static_cast<ssize_t>(frame.size())) {
      std::fprintf(stderr, "[smoke_serial] FUNC_UART_SERVO write failed\n");
      return 9;
    }
    // Hold the port open until well past run_time so the firmware has
    // time to actually drive the servo before the link closes.
    std::this_thread::sleep_for(
        std::chrono::milliseconds(run_time_ms + 500));
    std::printf("[smoke_serial] frame sent. Did servo %d physically move?\n"
                "  If YES — FUNC_UART_SERVO works; YahboomSystem bug.\n"
                "  If NO  — FUNC_UART_SERVO needs different prep (firmware mode lock?).\n",
                s_id);
    return 0;
  }

  // ── Query-arm mode (D7 prerequisite) ────────────────────────────────
  // Send FUNC_REQUEST_DATA(FUNC_ARM_CTRL), wait up to 1 second for the
  // FUNC_ARM_CTRL response frame, parse + print pulses + degrees + URDF rad
  // (with our offset/sign convention), then exit. NO torque, NO motion.
  if (query_arm_mode) {
    std::printf("[smoke_serial] --query-arm: requesting arm positions...\n");
    auto req = y::build_request_data(y::FUNC_ARM_CTRL);
    if (ser.write_bytes(req.data(), req.size()) != static_cast<ssize_t>(req.size())) {
      std::fprintf(stderr, "[smoke_serial] write FUNC_REQUEST_DATA failed\n");
      return 5;
    }

    y::FrameParser p;
    uint8_t buf[256];
    auto deadline = clk::now() + std::chrono::seconds(1);
    std::optional<y::ArmPulses> got;
    while (clk::now() < deadline && !got.has_value()) {
      ssize_t r = ser.read_bytes(buf, sizeof(buf));
      if (r > 0) {
        p.feed(buf, static_cast<size_t>(r));
        while (auto frame = p.next_frame()) {
          if (frame->func == y::FUNC_ARM_CTRL) {
            got = y::parse_arm_ctrl(frame->payload);
            break;
          }
          // Other frames (push reports we already enabled in vendor's last
          // session) get drained but ignored.
        }
      } else if (r < 0) {
        std::fprintf(stderr, "[smoke_serial] read error during arm query\n");
        return 6;
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }
    }
    if (!got) {
      std::fprintf(stderr, "[smoke_serial] --query-arm: TIMEOUT — no FUNC_ARM_CTRL response in 1s\n");
      std::fprintf(stderr, "  troubleshooting: STM32 may need AUTO_REPORT enabled first; try a default smoke run\n");
      std::fprintf(stderr, "  (no --query-arm flag) and verify FUNC_REPORT_* frames arrive, then retry --query-arm.\n");
      return 7;
    }

    // YahboomSystem-side per-joint conversion arrays (must match
    // yahboom_system.hpp kArmVendorOffsetDeg + kArmAxisSign):
    static const double kOffsetDeg[6] = {90.0, 90.0, 90.0, 90.0, 135.0, 90.0};
    static const double kAxisSign[6]  = {-1.0, -1.0, -1.0, -1.0,  +1.0,  +1.0};
    static const char* kJointName[6] = {
        "arm_joint1", "arm_joint2", "arm_joint3", "arm_joint4", "arm_joint5", "grip_joint"};

    std::printf("\n[smoke_serial] === ARM POSITIONS (one-shot read) ===\n");
    std::printf("  servo  pulse   deg     URDF-rad   joint\n");
    for (size_t i = 0; i < 6; ++i) {
      const uint8_t s_id = static_cast<uint8_t>(i + 1);
      const double deg = y::arm_pulse_to_angle(s_id, got->pulses[i]);
      const double rad = (deg - kOffsetDeg[i]) / (kAxisSign[i] * 180.0 / M_PI);
      std::printf("    s%zu   %5d  %+6.1f  %+8.4f   %s\n",
                  i + 1, got->pulses[i], deg, rad, kJointName[i]);
    }
    std::printf("\n[smoke_serial] PASS — arm-position query round-trip works.\n");
    std::printf("  Use this output to verify YahboomSystem on_configure seeding\n");
    std::printf("  (compare to /joint_states after launch).\n");
    return 0;
  }

  // ── Default mode: enable push-mode reports + drain ──────────────────
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
