// D2.3 wheel calibration tool — interactive operator-in-the-loop session.
//
// Two calibrations in one run, no ros2_control launch required (uses raw
// serial + protocol primitives directly):
//
//   Part A — wheel↔motor mapping
//     For each motor id 1..4: drive that motor at low duty for 2 s while
//     all others are commanded zero. Capture per-wheel encoder deltas.
//     The wheel index with the LARGEST delta is the one the motor drives.
//     Operator also confirms visually which wheel they saw spinning.
//
//   Part B — encoder counts per revolution
//     Operator picks one wheel, hand-rotates it exactly 1 full revolution.
//     Tool captures count delta = counts/rev for that wheel.
//
// Run on the Jetson container with /dev/myserial free (no ros2_control or
// vendor driver running):
//   ros2 run yahboom_ros2_control wheel_calibrate
//   ros2 run yahboom_ros2_control wheel_calibrate /dev/myserial
//
// Pre-conditions:
//   - Chassis on stand (wheels off the ground) — wheels will spin briefly
//   - Operator in front of robot to observe which physical wheel turns
//   - No other process holding the serial port

#include "yahboom_ros2_control/yahboom_protocol.hpp"
#include "yahboom_ros2_control/yahboom_serial.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace y = yahboom_ros2_control::protocol;
using yahboom_ros2_control::YahboomSerial;
using clk = std::chrono::steady_clock;
using std::printf;

namespace {

constexpr int kSpinDuty = 30;              // [-100..100], gentle but visible
constexpr int kSpinSeconds = 2;            // duration per motor
constexpr int kHeartbeatMs = 50;           // re-send FUNC_MOTOR while spinning
constexpr int kEncoderSeedTimeoutS = 3;    // wait this long for first encoder push

// Drain serial buffer into the parser, return latest encoder counts seen
// (returns std::nullopt if no encoder frame yet). Updates `latest`.
bool drain_one_encoder_frame(YahboomSerial& ser, y::FrameParser& parser,
                             y::EncoderCounts& latest) {
  bool got = false;
  uint8_t buf[256];
  while (true) {
    ssize_t r = ser.read_bytes(buf, sizeof(buf));
    if (r <= 0) break;
    parser.feed(buf, static_cast<size_t>(r));
  }
  while (auto f = parser.next_frame()) {
    if (f->func == y::FUNC_REPORT_ENCODER) {
      if (auto c = y::parse_encoder(f->payload)) {
        latest = *c;
        got = true;
      }
    }
  }
  return got;
}

// Block until at least one new encoder frame arrives (or timeout).
bool wait_for_encoder_frame(YahboomSerial& ser, y::FrameParser& parser,
                            y::EncoderCounts& latest, int timeout_s) {
  auto deadline = clk::now() + std::chrono::seconds(timeout_s);
  while (clk::now() < deadline) {
    if (drain_one_encoder_frame(ser, parser, latest)) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return false;
}

// Send FUNC_MOTOR with one motor at the given duty, all others zero.
void send_one_motor(YahboomSerial& ser, int motor_idx_1based, int duty) {
  std::array<int, 4> s{0, 0, 0, 0};
  s[motor_idx_1based - 1] = duty;
  auto frame = y::build_set_motor(s[0], s[1], s[2], s[3]);
  ser.write_bytes(frame.data(), frame.size());
}

void zero_motors(YahboomSerial& ser) {
  auto frame = y::build_set_motor(0, 0, 0, 0);
  ser.write_bytes(frame.data(), frame.size());
}

void prompt(const std::string& msg) {
  printf("\n>>> %s\n>>> Press ENTER when ready... ", msg.c_str());
  std::fflush(stdout);
  std::string line;
  std::getline(std::cin, line);
}

int32_t encoder_value(const y::EncoderCounts& c, int motor_idx_1based) {
  switch (motor_idx_1based) {
    case 1: return c.m1;
    case 2: return c.m2;
    case 3: return c.m3;
    case 4: return c.m4;
    default: return 0;
  }
}

const char* idx_to_wheel_name(size_t idx) {
  switch (idx) {
    case 0: return "front_left";
    case 1: return "front_right";
    case 2: return "rear_left";
    case 3: return "rear_right";
    default: return "?";
  }
}

}  // namespace

int main(int argc, char** argv) {
  std::string port = (argc > 1) ? argv[1] : "/dev/myserial";

  printf("=== D2.3 wheel calibration tool ===\n");
  printf("Port: %s\n\n", port.c_str());

  YahboomSerial ser;
  if (!ser.open(port)) {
    fprintf(stderr, "FAIL — could not open %s. Is ros2_control_node running? "
                    "Stop it first.\n", port.c_str());
    return 1;
  }
  printf("Serial open OK.\n");

  // Set car type + enable push reports
  {
    auto f1 = y::build_set_car_type(y::CAR_X3_PLUS);
    ser.write_bytes(f1.data(), f1.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto f2 = y::build_set_auto_report_state(true, false);
    ser.write_bytes(f2.data(), f2.size());
  }

  y::FrameParser parser;
  y::EncoderCounts latest;
  printf("Waiting for first encoder push frame... ");
  std::fflush(stdout);
  if (!wait_for_encoder_frame(ser, parser, latest, kEncoderSeedTimeoutS)) {
    fprintf(stderr, "TIMEOUT (no FUNC_REPORT_ENCODER in %d s). Is the STM32 "
                    "powered? Wrong port?\n", kEncoderSeedTimeoutS);
    return 2;
  }
  printf("OK. Initial counts: m1=%d m2=%d m3=%d m4=%d\n",
         latest.m1, latest.m2, latest.m3, latest.m4);

  // ─── Part A: wheel↔motor mapping ──────────────────────────────────────
  printf("\n========================================\n");
  printf("PART A — wheel/motor mapping\n");
  printf("========================================\n");
  printf("I will spin each motor (1..4) at duty %d for %d seconds while\n",
         kSpinDuty, kSpinSeconds);
  printf("all others are commanded zero. Watch which physical wheel turns\n");
  printf("each time. The motor index that drives a given wheel position\n");
  printf("(front_left / front_right / rear_left / rear_right) becomes the\n");
  printf("authoritative motor_id_for_wheel mapping.\n");

  prompt("Operator: confirm chassis is on a stand (wheels off ground).");

  // motor index 1..4 -> max encoder delta seen across all 4 wheels
  // motor_idx → which wheel's encoder moved the most
  std::array<size_t, 4> driven_wheel_for_motor{};
  std::array<int32_t, 4> driven_delta_for_motor{};
  std::array<std::string, 4> operator_observed_wheel{};

  for (int motor = 1; motor <= 4; ++motor) {
    char buf[64];
    std::snprintf(buf, sizeof(buf),
                  "About to spin MOTOR %d for %d s. Watch carefully.",
                  motor, kSpinSeconds);
    prompt(buf);

    // capture initial counts
    drain_one_encoder_frame(ser, parser, latest);
    const y::EncoderCounts before = latest;

    printf("[motor %d spinning duty=%d]\n", motor, kSpinDuty);
    auto end_time = clk::now() + std::chrono::seconds(kSpinSeconds);
    while (clk::now() < end_time) {
      send_one_motor(ser, motor, kSpinDuty);
      drain_one_encoder_frame(ser, parser, latest);
      std::this_thread::sleep_for(std::chrono::milliseconds(kHeartbeatMs));
    }
    zero_motors(ser);

    // Let counts settle for 200 ms (last encoder push to arrive)
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    drain_one_encoder_frame(ser, parser, latest);
    const y::EncoderCounts after = latest;

    int32_t deltas[4] = {
        std::abs(after.m1 - before.m1),
        std::abs(after.m2 - before.m2),
        std::abs(after.m3 - before.m3),
        std::abs(after.m4 - before.m4),
    };
    int max_idx = 0;
    for (int i = 1; i < 4; ++i) if (deltas[i] > deltas[max_idx]) max_idx = i;
    driven_wheel_for_motor[motor - 1] = static_cast<size_t>(max_idx);
    driven_delta_for_motor[motor - 1] = deltas[max_idx];

    printf("  encoder deltas: m1=%d m2=%d m3=%d m4=%d → motor %d drove "
           "encoder index %d (= '%s' under our default URDF order)\n",
           deltas[0], deltas[1], deltas[2], deltas[3],
           motor, max_idx, idx_to_wheel_name(static_cast<size_t>(max_idx)));

    printf("  [operator] which physical wheel did you SEE spinning?\n");
    printf("    F = front_left,  f = front_right,  R = rear_left,  r = rear_right\n");
    printf("    (or 'skip' if not sure):  ");
    std::fflush(stdout);
    std::string ans;
    std::getline(std::cin, ans);
    operator_observed_wheel[motor - 1] = ans;
  }

  // ─── Part B: counts per revolution ────────────────────────────────────
  printf("\n========================================\n");
  printf("PART B — encoder counts per revolution\n");
  printf("========================================\n");
  printf("Pick a wheel — recommended: front_left (motor 1). Rotate it\n");
  printf("BY HAND through exactly ONE full revolution. I will measure\n");
  printf("the count delta on each motor encoder.\n");
  printf("\n");
  printf("Tip: mark a paint-stripe on the wheel hub; align with chassis edge\n");
  printf("at start, rotate slowly until the stripe returns to the same edge.\n");

  prompt("Step 1: align your reference mark and confirm the wheel is at REST.");

  drain_one_encoder_frame(ser, parser, latest);
  const y::EncoderCounts rev_before = latest;
  printf("Counts AT START: m1=%d m2=%d m3=%d m4=%d\n",
         rev_before.m1, rev_before.m2, rev_before.m3, rev_before.m4);

  prompt("Step 2: now rotate the wheel exactly ONE FULL revolution by hand. "
         "Press ENTER when done.");

  drain_one_encoder_frame(ser, parser, latest);
  const y::EncoderCounts rev_after = latest;

  int32_t rev_delta[4] = {
      std::abs(rev_after.m1 - rev_before.m1),
      std::abs(rev_after.m2 - rev_before.m2),
      std::abs(rev_after.m3 - rev_before.m3),
      std::abs(rev_after.m4 - rev_before.m4),
  };

  printf("Counts AT END  : m1=%d m2=%d m3=%d m4=%d\n",
         rev_after.m1, rev_after.m2, rev_after.m3, rev_after.m4);
  printf("Deltas (= counts / 1 rev): m1=%d m2=%d m3=%d m4=%d\n",
         rev_delta[0], rev_delta[1], rev_delta[2], rev_delta[3]);

  int max_idx = 0;
  for (int i = 1; i < 4; ++i) if (rev_delta[i] > rev_delta[max_idx]) max_idx = i;
  int counts_per_rev = rev_delta[max_idx];

  // ─── Summary ──────────────────────────────────────────────────────────
  printf("\n========================================\n");
  printf("CALIBRATION SUMMARY\n");
  printf("========================================\n");
  printf("\n--- wheel/motor mapping (Part A) ---\n");
  for (int m = 1; m <= 4; ++m) {
    printf("  motor %d  drove wheel index %zu (%s, %d encoder counts) "
           "[operator saw: '%s']\n",
           m, driven_wheel_for_motor[m - 1],
           idx_to_wheel_name(driven_wheel_for_motor[m - 1]),
           driven_delta_for_motor[m - 1],
           operator_observed_wheel[m - 1].c_str());
  }
  printf("\n  Default code mapping (motor_id_for_wheel in yahboom_system.hpp):\n");
  printf("    motor 1 -> front_left (FL=0)\n");
  printf("    motor 2 -> front_right (FR=1)\n");
  printf("    motor 3 -> rear_left (RL=2)\n");
  printf("    motor 4 -> rear_right (RR=3)\n");
  printf("  Compare to measured above. Edit motor_id_for_wheel() if mismatched.\n");

  printf("\n--- counts per revolution (Part B) ---\n");
  printf("  Largest delta over 1 hand revolution: %d counts (motor %d encoder)\n",
         counts_per_rev, max_idx + 1);
  printf("  ⇒ encoder_counts_per_rev ≈ %d\n", counts_per_rev);
  printf("\n  Update URDF param:\n");
  printf("    <param name=\"encoder_counts_per_rev\">%d</param>\n", counts_per_rev);

  printf("\nDONE. Wheels commanded zero. You can now restart ros2_control launch.\n");

  zero_motors(ser);
  return 0;
}
