// ros2_control SystemInterface for the Yahboom ROSMASTER X3PLUS.
//
// D2 (chassis-only): 4 mecanum wheel velocity command interfaces +
// per-wheel position/velocity state interfaces from FUNC_REPORT_ENCODER push.
// Uses the Path A round-trip shortcut (read commanded wheel velocities,
// invert mecanum kinematics back to Twist, send FUNC_MOTION) so STM32
// firmware kinematics do the actual per-wheel duty conversion. Constants
// cancel: controller's per-wheel ω → our inverse → Twist on wire == Twist
// the operator sent.
//
// D3+ will add IMU + 6 arm joints; design accommodates expansion via the
// same SystemInterface.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "yahboom_ros2_control/yahboom_protocol.hpp"
#include "yahboom_ros2_control/yahboom_serial.hpp"

namespace yahboom_ros2_control {

class YahboomSystem : public hardware_interface::SystemInterface {
 public:
  hardware_interface::CallbackReturn on_init(
      const hardware_interface::HardwareInfo& info) override;

  hardware_interface::CallbackReturn on_configure(
      const rclcpp_lifecycle::State& previous_state) override;

  hardware_interface::CallbackReturn on_activate(
      const rclcpp_lifecycle::State& previous_state) override;

  hardware_interface::CallbackReturn on_deactivate(
      const rclcpp_lifecycle::State& previous_state) override;

  hardware_interface::CallbackReturn on_cleanup(
      const rclcpp_lifecycle::State& previous_state) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type read(const rclcpp::Time& time,
                                       const rclcpp::Duration& period) override;
  hardware_interface::return_type write(const rclcpp::Time& time,
                                        const rclcpp::Duration& period) override;

 private:
  // Wheel ordering (FL, FR, RL, RR) — matches mecanum_drive_controller's
  // declaration order in ros2_controllers.yaml. Indices 0..3 throughout.
  static constexpr size_t FL = 0;
  static constexpr size_t FR = 1;
  static constexpr size_t RL = 2;
  static constexpr size_t RR = 3;
  static constexpr size_t NUM_WHEELS = 4;

  // ── Wheel position → STM32 motor/encoder index mapping ──
  // VERIFIED 2026-05-05 via wheel_calibrate D2.3 session:
  //   motor 1 drives front_left  (encoder m1)
  //   motor 2 drives REAR_LEFT   (encoder m2)  ← was incorrectly assumed FR
  //   motor 3 drives FRONT_RIGHT (encoder m3)  ← was incorrectly assumed RL
  //   motor 4 drives rear_right  (encoder m4)
  // Yahboom's physical wiring puts motors 2 and 3 swapped relative to the
  // naive "1=FL, 2=FR, 3=RL, 4=RR" default. The kinematics math in write()
  // is wheel-position-based and unaffected (FUNC_MOTION takes Twist, STM32
  // knows its own layout). Only the read-back from FUNC_REPORT_ENCODER
  // needs the index swap to populate state interfaces correctly.
  static constexpr std::array<int, NUM_WHEELS> kMotorIdForWheel = {
      1,  // FL → STM32 motor 1
      3,  // FR → STM32 motor 3 (NOT 2)
      2,  // RL → STM32 motor 2 (NOT 3)
      4,  // RR → STM32 motor 4
  };
  // Index into FUNC_REPORT_ENCODER frame (m1@0, m2@1, m3@2, m4@3) for a
  // given wheel position — same mapping as above, 0-based.
  static constexpr std::array<int, NUM_WHEELS> kEncoderIndexForWheel = {
      0,  // FL ← m1 (frame index 0)
      2,  // FR ← m3 (frame index 2)
      1,  // RL ← m2 (frame index 1)
      3,  // RR ← m4 (frame index 3)
  };
  static constexpr int motor_id_for_wheel(size_t w) {
    return kMotorIdForWheel[w];
  }
  static constexpr int encoder_index_for_wheel(size_t w) {
    return kEncoderIndexForWheel[w];
  }

  // ── Hardware params (from URDF <hardware><param>...</param></hardware>) ──
  std::string serial_port_ = "/dev/myserial";
  double wheel_radius_ = 0.040;            // m. 80 mm wheel (vendor schematic).
  double wheel_separation_x_ = 0.220;      // m. front↔rear axle (vendor schematic).
  double wheel_separation_y_ = 0.2082;     // m. left↔right axle (245.60 − 37.40, schematic).
  double encoder_counts_per_rev_ = 2436.0; // verified 2026-05-05 via wheel_calibrate hand-rev test (≈ 11 PPR × 56:1 × 4 quadrature = 2464 nominal).
  bool dry_run_ = false;

  // ── Runtime state ──
  YahboomSerial serial_;
  protocol::FrameParser parser_;

  // Joint state arrays (index = wheel position FL/FR/RL/RR).
  std::array<double, NUM_WHEELS> wheel_position_state_{};   // rad, accumulated
  std::array<double, NUM_WHEELS> wheel_velocity_state_{};   // rad/s, instantaneous
  std::array<double, NUM_WHEELS> wheel_velocity_command_{}; // rad/s, from controller

  // Encoder bookkeeping for velocity calc + position integration.
  std::array<int32_t, NUM_WHEELS> last_encoder_counts_{};
  bool encoder_seeded_ = false;
  rclcpp::Time last_encoder_time_{0, 0, RCL_ROS_TIME};

  // FUNC_MOTION send throttling. STM32 firmware can't keep up with 100 Hz
  // FUNC_MOTION frames — each new frame appears to interrupt the previous,
  // net result is ~70x slowdown observed during D2 first live test.
  // Vendor sends event-driven (~10-30 Hz). We dedupe by Twist value and
  // heartbeat every kMotionHeartbeatMs to stay above any STM32 watchdog.
  static constexpr double kMotionEpsilon = 1e-4;       // m/s or rad/s
  static constexpr int64_t kMotionHeartbeatMs = 100;   // re-send anyway every 100 ms
  double last_sent_vx_ = 0.0;
  double last_sent_vy_ = 0.0;
  double last_sent_wz_ = 0.0;
  rclcpp::Time last_motion_send_time_{0, 0, RCL_ROS_TIME};
  bool motion_send_seeded_ = false;

  // URDF joint name lookup — tells us which wheel slot each export goes to.
  // Filled in on_init from info_.joints.
  std::array<std::string, NUM_WHEELS> wheel_joint_names_{};

  // Helpers
  void zero_wheel_command();
  void send_motion_command(double vx, double vy, double wz);
  void update_state_from_encoder_frame(const protocol::EncoderCounts& counts,
                                       const rclcpp::Time& now);
};

}  // namespace yahboom_ros2_control
