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
#include <optional>
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

  // ── Arm joints (D4) ──
  // 6 bus servos, addressed by STM32 servo id 1..6 (natural order; NOT
  // reversed like Ultra). URDF joints declared in this order:
  //   index 0 = arm_joint1 (base yaw)
  //   index 1 = arm_joint2 (shoulder)
  //   index 2 = arm_joint3 (elbow)
  //   index 3 = arm_joint4 (wrist pitch)
  //   index 4 = arm_joint5 (wrist rot, asymmetric 0..270° range)
  //   index 5 = grip_joint (gripper, vendor-protected 30° lower bound)
  static constexpr size_t NUM_ARM_JOINTS = 6;
  static constexpr int servo_id_for_arm_joint(size_t j) {
    return static_cast<int>(j) + 1;  // 1-indexed
  }

  // URDF rad ↔ vendor deg conversion: vendor_deg = offset + sign * rad * 180/π.
  // Hardcoded defaults; refine at D7 calibration. Joints 1-4 have axisZ=-1
  // in URDF (so positive rad → negative vendor angle change); joints 5 and
  // grip_joint have axisZ=+1.
  // Offsets: vendor angle when URDF rad = 0 (URDF "zero pose").
  // - joints 1-4: offset 90° puts URDF zero at vendor mid-range
  // - joint 5: offset 135° (vendor range 0..270 — mid is 135)
  // - grip_joint: offset 90° (placeholder; vendor "closed" is 30° per
  //   yahboom_joy_X3plus.py:212, vendor "open" is 180°; URDF range -π/2..0
  //   maps onto a sub-range of vendor 0..180 — TODO_CALIBRATE at D7)
  static constexpr std::array<double, NUM_ARM_JOINTS> kArmVendorOffsetDeg = {
      90.0,   // arm_joint1
      90.0,   // arm_joint2
      90.0,   // arm_joint3
      90.0,   // arm_joint4
      135.0,  // arm_joint5
      90.0,   // grip_joint  TODO_CALIBRATE
  };
  static constexpr std::array<double, NUM_ARM_JOINTS> kArmAxisSign = {
      -1.0, -1.0, -1.0, -1.0, +1.0, +1.0,
  };

  // URDF position limits (rad) per arm joint, from
  // yahboomcar_X3plus.urdf.xacro (verified D2.3). on_activate refuses to
  // engage if a freshly-queried joint state lies outside this range —
  // operator must back-drive the offending joint into URDF range first
  // (use `smoke_serial /dev/myserial --torque-off` to disengage). This
  // prevents the "first write commands a state ros2_control thinks is
  // impossible" failure mode.
  static constexpr std::array<double, NUM_ARM_JOINTS> kArmUrdfLo = {
      -1.5708, -1.5708, -1.5708, -1.5708, -1.5708, -1.5708,  // -π/2 except joint5/grip already noted below
  };
  static constexpr std::array<double, NUM_ARM_JOINTS> kArmUrdfHi = {
      +1.5708, +1.5708, +1.5708, +1.5708, +3.1416,  0.0,     // +π/2 for joints 1-4; +π for joint5; 0 for grip
  };

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

  // ── IMU state interfaces (D3) ──
  // Populated from FUNC_REPORT_IMU_ATT (roll/pitch/yaw → quaternion) and
  // FUNC_REPORT_ICM_RAW (gyro = angular velocity rad/s, accel = linear
  // acceleration m/s²). 10 doubles match imu_sensor_broadcaster's required
  // state-interface set. Magnetometer is decoded but not exposed (broadcaster
  // doesn't consume it; would need a separate magnetometer_broadcaster).
  std::string imu_sensor_name_ = "imu_sensor";  // overridden from URDF
  double imu_orientation_x_ = 0.0;
  double imu_orientation_y_ = 0.0;
  double imu_orientation_z_ = 0.0;
  double imu_orientation_w_ = 1.0;              // identity quaternion at init
  double imu_angular_velocity_x_ = 0.0;
  double imu_angular_velocity_y_ = 0.0;
  double imu_angular_velocity_z_ = 0.0;
  double imu_linear_acceleration_x_ = 0.0;
  double imu_linear_acceleration_y_ = 0.0;
  double imu_linear_acceleration_z_ = 0.0;

  // URDF joint name lookup — tells us which wheel slot each export goes to.
  // Filled in on_init from info_.joints.
  std::array<std::string, NUM_WHEELS> wheel_joint_names_{};

  // ── Arm joints (D4) ──
  std::array<std::string, NUM_ARM_JOINTS> arm_joint_names_{};
  bool arm_present_ = false;  // true if URDF declares 6 arm joints after the wheels

  std::array<double, NUM_ARM_JOINTS> arm_position_state_{};      // rad (URDF convention)
  std::array<double, NUM_ARM_JOINTS> arm_position_command_{};    // rad (URDF convention)
  // Last sent vendor angle (deg), used for write-side dedupe + heartbeat —
  // STM32 firmware likely has the same per-frame issue arm side as chassis,
  // so we apply Twist-style throttling at the FUNC_ARM_CTRL level.
  std::array<double, NUM_ARM_JOINTS> last_sent_arm_deg_{};
  rclcpp::Time last_arm_send_time_{0, 0, RCL_ROS_TIME};
  bool arm_send_seeded_ = false;
  static constexpr double kArmDegEpsilon = 0.5;             // ≈ 9 mrad
  static constexpr int64_t kArmHeartbeatMs = 100;

  // Helpers
  void zero_wheel_command();
  void send_motion_command(double vx, double vy, double wz);
  void update_state_from_encoder_frame(const protocol::EncoderCounts& counts,
                                       const rclcpp::Time& now);
  // roll/pitch/yaw → quaternion (z-y-x intrinsic, ROS REP-103 convention).
  static void rpy_to_quat(double roll, double pitch, double yaw,
                          double& qx, double& qy, double& qz, double& qw);
  // URDF rad → vendor degrees per joint (uses kArmVendorOffsetDeg + kArmAxisSign).
  static double urdf_rad_to_vendor_deg(size_t arm_joint_idx, double rad);
  // Inverse: vendor pulse (raw STM32 units) → URDF rad. Inverts the chain
  // arm_pulse_to_angle → (deg - offset) / (sign * 180/π).
  static double vendor_pulse_to_urdf_rad(size_t arm_joint_idx, int16_t pulse);

  // Synchronous arm-position read: write FUNC_REQUEST_DATA(FUNC_ARM_CTRL),
  // wait up to 1s for the FUNC_ARM_CTRL response, return the 6 pulses on
  // success or nullopt on timeout/IO failure. Uses a LOCAL parser so any
  // push-mode frames received during the wait are dropped (the read() loop
  // picks them up fresh on the next cycle). Safe to call from on_configure
  // / on_activate (no torque change, no motion command).
  std::optional<protocol::ArmPulses> query_arm_positions();

  // Query arm positions, validate against [kArmUrdfLo, kArmUrdfHi], and on
  // success seed arm_position_state_ + arm_position_command_ from the
  // measured pose so the first write() doesn't lurch. Returns false (and
  // logs which joint(s) failed) if the query times out OR any joint reads
  // outside the URDF range — operator must back-drive into range first
  // (use `smoke_serial /dev/myserial --torque-off`). Sole point of arm
  // seeding for both on_configure and on_activate.
  bool validate_and_seed_arm_from_hardware();
};

}  // namespace yahboom_ros2_control
