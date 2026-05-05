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

  // Wheel name → STM32 motor ID (1..4 for FUNC_MOTOR / FUNC_REPORT_ENCODER).
  // TODO_VERIFY: confirm physical mapping by spinning each motor in isolation.
  // Initial guess: m1=FL, m2=FR, m3=RL, m4=RR (vendor convention typical).
  static constexpr int motor_id_for_wheel(size_t w) {
    return static_cast<int>(w) + 1;  // 1-indexed
  }

  // ── Hardware params (from URDF <hardware><param>...</param></hardware>) ──
  std::string serial_port_ = "/dev/myserial";
  double wheel_radius_ = 0.040;            // m. 80 mm wheel (vendor schematic).
  double wheel_separation_x_ = 0.220;      // m. front↔rear axle (vendor schematic).
  double wheel_separation_y_ = 0.2082;     // m. left↔right axle (245.60 − 37.40, schematic).
  double encoder_counts_per_rev_ = 616.0;  // 11 Hall PPR × 56:1 gearbox, TODO_VERIFY.
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
