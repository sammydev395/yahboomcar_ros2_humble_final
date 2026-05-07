// D2 chassis-only YahboomSystem implementation. See yahboom_system.hpp for
// architecture notes (Path A = round-trip cancellation via FUNC_MOTION).

#include "yahboom_ros2_control/yahboom_system.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <thread>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/logging.hpp"

namespace yahboom_ros2_control {

namespace {
constexpr const char* kLogger = "YahboomSystem";

// Read a string param from HardwareInfo, return default if missing.
std::string get_param(
    const std::unordered_map<std::string, std::string>& params,
    const std::string& key, const std::string& fallback) {
  auto it = params.find(key);
  return (it != params.end()) ? it->second : fallback;
}

double get_param_double(
    const std::unordered_map<std::string, std::string>& params,
    const std::string& key, double fallback) {
  auto it = params.find(key);
  if (it == params.end()) return fallback;
  try {
    return std::stod(it->second);
  } catch (const std::exception&) {
    return fallback;
  }
}

bool get_param_bool(
    const std::unordered_map<std::string, std::string>& params,
    const std::string& key, bool fallback) {
  auto it = params.find(key);
  if (it == params.end()) return fallback;
  std::string v = it->second;
  std::transform(v.begin(), v.end(), v.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return (v == "true" || v == "1" || v == "yes");
}
}  // namespace

// ─── Lifecycle ───────────────────────────────────────────────────────────────

hardware_interface::CallbackReturn YahboomSystem::on_init(
    const hardware_interface::HardwareInfo& info) {
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  const auto& p = info_.hardware_parameters;
  serial_port_              = get_param(p, "serial_port", serial_port_);
  wheel_radius_             = get_param_double(p, "wheel_radius", wheel_radius_);
  wheel_separation_x_       = get_param_double(p, "wheel_separation_x", wheel_separation_x_);
  wheel_separation_y_       = get_param_double(p, "wheel_separation_y", wheel_separation_y_);
  encoder_counts_per_rev_   = get_param_double(p, "encoder_counts_per_rev", encoder_counts_per_rev_);
  dry_run_                  = get_param_bool(p, "dry_run", dry_run_);

  RCLCPP_INFO(rclcpp::get_logger(kLogger),
              "on_init: serial_port=%s wheel_radius=%.4f sep_x=%.4f sep_y=%.4f "
              "counts/rev=%.0f dry_run=%s",
              serial_port_.c_str(), wheel_radius_, wheel_separation_x_,
              wheel_separation_y_, encoder_counts_per_rev_,
              dry_run_ ? "true" : "false");

  // URDF declares either 4 wheel joints (chassis-only) or 4 wheel + 6 arm
  // joints (full chassis+arm). Wheels first (positions 0..3), arm next
  // (positions 4..9). Anything else is a config error.
  if (info_.joints.size() != NUM_WHEELS &&
      info_.joints.size() != NUM_WHEELS + NUM_ARM_JOINTS) {
    RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                 "expected %zu (chassis-only) or %zu (chassis+arm) joints in URDF, got %zu",
                 NUM_WHEELS, NUM_WHEELS + NUM_ARM_JOINTS, info_.joints.size());
    return hardware_interface::CallbackReturn::ERROR;
  }
  arm_present_ = (info_.joints.size() == NUM_WHEELS + NUM_ARM_JOINTS);

  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    wheel_joint_names_[i] = info_.joints[i].name;
    // Each wheel must declare exactly: 1 velocity command + position + velocity state.
    bool has_vel_cmd = false, has_pos_state = false, has_vel_state = false;
    for (const auto& iface : info_.joints[i].command_interfaces) {
      if (iface.name == hardware_interface::HW_IF_VELOCITY) has_vel_cmd = true;
    }
    for (const auto& iface : info_.joints[i].state_interfaces) {
      if (iface.name == hardware_interface::HW_IF_POSITION) has_pos_state = true;
      if (iface.name == hardware_interface::HW_IF_VELOCITY) has_vel_state = true;
    }
    if (!has_vel_cmd || !has_pos_state || !has_vel_state) {
      RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                   "wheel joint '%s' missing required interfaces "
                   "(need: cmd=velocity, state=position+velocity)",
                   wheel_joint_names_[i].c_str());
      return hardware_interface::CallbackReturn::ERROR;
    }
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "wheel[%zu] = '%s' → STM32 motor id %d (encoder index %d)",
                i, wheel_joint_names_[i].c_str(),
                motor_id_for_wheel(i), encoder_index_for_wheel(i));
  }

  if (arm_present_) {
    for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
      const auto& joint = info_.joints[NUM_WHEELS + j];
      arm_joint_names_[j] = joint.name;
      // Each arm joint declares: 1 position command + 1 position state.
      bool has_pos_cmd = false, has_pos_state = false;
      for (const auto& iface : joint.command_interfaces) {
        if (iface.name == hardware_interface::HW_IF_POSITION) has_pos_cmd = true;
      }
      for (const auto& iface : joint.state_interfaces) {
        if (iface.name == hardware_interface::HW_IF_POSITION) has_pos_state = true;
      }
      if (!has_pos_cmd || !has_pos_state) {
        RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                     "arm joint '%s' missing required interfaces "
                     "(need: cmd=position, state=position)",
                     arm_joint_names_[j].c_str());
        return hardware_interface::CallbackReturn::ERROR;
      }
      RCLCPP_INFO(rclcpp::get_logger(kLogger),
                  "arm[%zu] = '%s' → STM32 servo id %d "
                  "(URDF rad=0 → vendor %.1f°, axis_sign=%+0.0f)",
                  j, arm_joint_names_[j].c_str(), servo_id_for_arm_joint(j),
                  kArmVendorOffsetDeg[j], kArmAxisSign[j]);
    }
  } else {
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "URDF has no arm joints — chassis-only mode");
  }

  // ── D3: IMU sensor declaration ──
  // URDF may include zero or one <sensor name="..."> elements inside the
  // <ros2_control> block. If present, capture the name for state-interface
  // export and verify it declares the 10 standard IMU fields.
  if (!info_.sensors.empty()) {
    if (info_.sensors.size() > 1) {
      RCLCPP_WARN(rclcpp::get_logger(kLogger),
                  "expected ≤1 sensor in URDF, got %zu — using first ('%s')",
                  info_.sensors.size(), info_.sensors[0].name.c_str());
    }
    imu_sensor_name_ = info_.sensors[0].name;
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "IMU sensor declared: name='%s' with %zu state interfaces",
                imu_sensor_name_.c_str(),
                info_.sensors[0].state_interfaces.size());
  } else {
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "no IMU sensor declared in URDF — not exporting IMU state. "
                "(Add <sensor name=\"imu_sensor\"> block to enable.)");
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_configure(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  if (dry_run_) {
    RCLCPP_WARN(rclcpp::get_logger(kLogger),
                "DRY RUN: not opening %s, no STM32 frames will be written",
                serial_port_.c_str());
    return hardware_interface::CallbackReturn::SUCCESS;
  }

  if (!serial_.open(serial_port_)) {
    RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                 "failed to open %s — check that vendor Mcnamu_driver_X3plus "
                 "or rosmaster_main.py isn't holding the port",
                 serial_port_.c_str());
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Set car type (defensive — STM32 may have a stale setting from a prior
  // session). Forever flag = false: RAM-only, doesn't write flash.
  {
    auto frame = protocol::build_set_car_type(protocol::CAR_X3_PLUS);
    if (serial_.write_bytes(frame.data(), frame.size()) !=
        static_cast<ssize_t>(frame.size())) {
      RCLCPP_ERROR(rclcpp::get_logger(kLogger), "SET_CAR_TYPE write failed");
      serial_.close();
      return hardware_interface::CallbackReturn::ERROR;
    }
  }

  // Enable push-mode reports (encoder/IMU/speed at ~27 Hz each).
  {
    auto frame = protocol::build_set_auto_report_state(true, false);
    if (serial_.write_bytes(frame.data(), frame.size()) !=
        static_cast<ssize_t>(frame.size())) {
      RCLCPP_ERROR(rclcpp::get_logger(kLogger), "AUTO_REPORT write failed");
      serial_.close();
      return hardware_interface::CallbackReturn::ERROR;
    }
  }

  parser_.reset();
  encoder_seeded_ = false;

  // ── D7: arm position seed (LIVE only) ──
  // Query current arm pulses, validate they're inside URDF range, and seed
  // arm_position_state_/arm_position_command_ from the measured pose. This
  // guarantees the controller's first reference matches the physical pose
  // (no first-write lurch — TELEOP_PHASE4_LESSONS.md lessons 1-4).
  // Refusal-to-configure if any joint reads outside URDF range is the
  // explicit safety check; operator must back-drive into range first.
  if (arm_present_) {
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "on_configure: querying arm positions for seed + URDF-range check");
    if (!validate_and_seed_arm_from_hardware()) {
      serial_.close();
      return hardware_interface::CallbackReturn::ERROR;
    }
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "arm seeded: cmd = state from hardware (first write == no-op)");
  }

  RCLCPP_INFO(rclcpp::get_logger(kLogger), "on_configure OK on %s", serial_port_.c_str());
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  // Defensive: zero command + state arrays so the first write() doesn't
  // command stale velocity. Same re-seed pattern as Ultra (TELEOP_PHASE4
  // lesson 4 — "on_activate MUST re-seed cmd from state").
  wheel_velocity_command_.fill(0.0);
  wheel_velocity_state_.fill(0.0);
  // wheel_position_state_ keeps its accumulated value (the physical wheel
  // didn't move while INACTIVE; encoder counts are still cumulative).

  zero_wheel_command();  // send a zero-Twist FUNC_MOTION right now

  // ── D7: arm activation (LIVE only) ──
  // 1. Re-query + re-validate + re-seed (operator might have back-driven the
  //    arm between configure and activate; trust nothing).
  // 2. Send a single FUNC_ARM_CTRL nailing each servo to its current pose
  //    (write() would do this on first call anyway, but emit one upfront
  //    so the firmware "hold target" is unambiguous).
  // 3. Send FUNC_UART_SERVO_TORQUE(true) — defensive. STM32 firmware boots
  //    with torque enabled by default per operator observation, but if the
  //    operator ran --torque-off to back-drive joints, we need to re-engage.
  //    Order matters: torque-on AFTER the position command so the firmware
  //    engages servos at the freshly-commanded current pose.
  if (!dry_run_ && arm_present_) {
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "on_activate: re-querying arm positions (defensive re-seed)");
    if (!validate_and_seed_arm_from_hardware()) {
      RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                   "on_activate: arm seed failed — refusing to activate");
      return hardware_interface::CallbackReturn::ERROR;
    }
    // Build + send a FUNC_ARM_CTRL frame at the seeded pose (cmd == state).
    std::array<double, 6> angles_deg{};
    for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
      angles_deg[j] = urdf_rad_to_vendor_deg(j, arm_position_command_[j]);
    }
    {
      auto frame = protocol::build_set_uart_servo_angle_array(angles_deg);
      if (serial_.write_bytes(frame.data(), frame.size()) !=
          static_cast<ssize_t>(frame.size())) {
        RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                     "on_activate: failed to write initial FUNC_ARM_CTRL");
        return hardware_interface::CallbackReturn::ERROR;
      }
    }
    {
      auto frame = protocol::build_set_uart_servo_torque(true);
      if (serial_.write_bytes(frame.data(), frame.size()) !=
          static_cast<ssize_t>(frame.size())) {
        RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                     "on_activate: failed to write FUNC_UART_SERVO_TORQUE(on)");
        return hardware_interface::CallbackReturn::ERROR;
      }
    }
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "arm torque ENABLED, holding seeded pose: "
                "deg=[%.1f %.1f %.1f %.1f %.1f %.1f]",
                angles_deg[0], angles_deg[1], angles_deg[2],
                angles_deg[3], angles_deg[4], angles_deg[5]);
  }

  RCLCPP_INFO(rclcpp::get_logger(kLogger),
              "on_activate OK (wheel commands zeroed%s)",
              (arm_present_ && !dry_run_) ? ", arm seeded + torque on" : "");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_deactivate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  wheel_velocity_command_.fill(0.0);
  zero_wheel_command();
  RCLCPP_INFO(rclcpp::get_logger(kLogger), "on_deactivate OK");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn YahboomSystem::on_cleanup(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  if (serial_.is_open()) {
    zero_wheel_command();
    serial_.close();
  }
  RCLCPP_INFO(rclcpp::get_logger(kLogger), "on_cleanup OK");
  return hardware_interface::CallbackReturn::SUCCESS;
}

// ─── Interface export ────────────────────────────────────────────────────────

std::vector<hardware_interface::StateInterface> YahboomSystem::export_state_interfaces() {
  std::vector<hardware_interface::StateInterface> out;
  out.reserve(2 * NUM_WHEELS + NUM_ARM_JOINTS + 10);
  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_POSITION,
                     &wheel_position_state_[i]);
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_VELOCITY,
                     &wheel_velocity_state_[i]);
  }

  // D4: arm joint position state (only if URDF declares arm joints).
  if (arm_present_) {
    for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
      out.emplace_back(arm_joint_names_[j],
                       hardware_interface::HW_IF_POSITION,
                       &arm_position_state_[j]);
    }
  }

  // D3: 10 IMU state interfaces (only if sensor declared in URDF).
  // Names are the convention imu_sensor_broadcaster expects.
  if (!info_.sensors.empty()) {
    out.emplace_back(imu_sensor_name_, "orientation.x",         &imu_orientation_x_);
    out.emplace_back(imu_sensor_name_, "orientation.y",         &imu_orientation_y_);
    out.emplace_back(imu_sensor_name_, "orientation.z",         &imu_orientation_z_);
    out.emplace_back(imu_sensor_name_, "orientation.w",         &imu_orientation_w_);
    out.emplace_back(imu_sensor_name_, "angular_velocity.x",    &imu_angular_velocity_x_);
    out.emplace_back(imu_sensor_name_, "angular_velocity.y",    &imu_angular_velocity_y_);
    out.emplace_back(imu_sensor_name_, "angular_velocity.z",    &imu_angular_velocity_z_);
    out.emplace_back(imu_sensor_name_, "linear_acceleration.x", &imu_linear_acceleration_x_);
    out.emplace_back(imu_sensor_name_, "linear_acceleration.y", &imu_linear_acceleration_y_);
    out.emplace_back(imu_sensor_name_, "linear_acceleration.z", &imu_linear_acceleration_z_);
  }
  return out;
}

std::vector<hardware_interface::CommandInterface> YahboomSystem::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> out;
  out.reserve(NUM_WHEELS + NUM_ARM_JOINTS);
  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_VELOCITY,
                     &wheel_velocity_command_[i]);
  }
  if (arm_present_) {
    for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
      out.emplace_back(arm_joint_names_[j],
                       hardware_interface::HW_IF_POSITION,
                       &arm_position_command_[j]);
    }
  }
  return out;
}

double YahboomSystem::urdf_rad_to_vendor_deg(size_t arm_joint_idx, double rad) {
  // vendor_deg = offset + sign * rad * 180/π
  return kArmVendorOffsetDeg[arm_joint_idx] +
         kArmAxisSign[arm_joint_idx] * rad * (180.0 / M_PI);
}

double YahboomSystem::vendor_pulse_to_urdf_rad(size_t arm_joint_idx, int16_t pulse) {
  // Inverse of urdf_rad_to_vendor_deg, but the pulse→deg step is asymmetric
  // per joint (handled by protocol::arm_pulse_to_angle).
  const uint8_t s_id = static_cast<uint8_t>(arm_joint_idx + 1);
  const double deg = protocol::arm_pulse_to_angle(s_id, pulse);
  return (deg - kArmVendorOffsetDeg[arm_joint_idx]) /
         (kArmAxisSign[arm_joint_idx] * (180.0 / M_PI));
}

std::optional<protocol::ArmPulses> YahboomSystem::query_arm_positions() {
  if (!serial_.is_open()) return std::nullopt;
  auto req = protocol::build_request_data(protocol::FUNC_ARM_CTRL);
  if (serial_.write_bytes(req.data(), req.size()) !=
      static_cast<ssize_t>(req.size())) {
    return std::nullopt;
  }
  // LOCAL parser so push frames received during the wait don't pollute
  // parser_ state. Cost: a handful of dropped push frames during the 1s
  // window — fine, the next read() cycle picks them up fresh.
  protocol::FrameParser local;
  uint8_t buf[256];
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (std::chrono::steady_clock::now() < deadline) {
    ssize_t r = serial_.read_bytes(buf, sizeof(buf));
    if (r > 0) {
      local.feed(buf, static_cast<size_t>(r));
      while (auto frame = local.next_frame()) {
        if (frame->func == protocol::FUNC_ARM_CTRL) {
          return protocol::parse_arm_ctrl(frame->payload);
        }
      }
    } else if (r < 0) {
      return std::nullopt;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }
  return std::nullopt;
}

bool YahboomSystem::validate_and_seed_arm_from_hardware() {
  auto pulses = query_arm_positions();
  if (!pulses) {
    RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                 "arm position query TIMEOUT (1s) — refusing to seed. "
                 "Is the STM32 responsive? Try `smoke_serial /dev/myserial` "
                 "(default mode) and confirm FUNC_REPORT_* frames arrive.");
    return false;
  }
  std::array<double, NUM_ARM_JOINTS> rads{};
  bool all_ok = true;
  for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
    rads[j] = vendor_pulse_to_urdf_rad(j, pulses->pulses[j]);
    const bool in_range =
        rads[j] >= kArmUrdfLo[j] && rads[j] <= kArmUrdfHi[j];
    RCLCPP_INFO(rclcpp::get_logger(kLogger),
                "  arm[%zu] '%s': pulse=%d urdf_rad=%+.4f "
                "(URDF [%+.4f, %+.4f]) %s",
                j, arm_joint_names_[j].c_str(), pulses->pulses[j], rads[j],
                kArmUrdfLo[j], kArmUrdfHi[j], in_range ? "OK" : "OUTSIDE");
    if (!in_range) all_ok = false;
  }
  if (!all_ok) {
    RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                 "one or more arm joints outside URDF range — refusing to "
                 "seed. Disable torque ('ros2 run yahboom_ros2_control "
                 "smoke_serial /dev/myserial --torque-off'), back-drive "
                 "the offending joint(s) into URDF range, then re-launch.");
    return false;
  }
  arm_position_state_ = rads;
  arm_position_command_ = rads;  // seed cmd = state → first write() == no-op
  // Reset write-side dedupe so the next write() emits the seeded pose
  // (don't carry stale last_sent_arm_deg_ from a prior configure cycle).
  arm_send_seeded_ = false;
  return true;
}

// ─── read() — drain push frames, update wheel state ──────────────────────────

hardware_interface::return_type YahboomSystem::read(const rclcpp::Time& time,
                                                     const rclcpp::Duration& /*period*/) {
  if (dry_run_) {
    // Perfect-tracker model: state echoes command. Used by D4 arm dry-run
    // bring-up so spawned controllers see "instant settle" and no faults.
    for (size_t i = 0; i < NUM_WHEELS; ++i) {
      wheel_velocity_state_[i] = wheel_velocity_command_[i];
    }
    if (arm_present_) {
      for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
        arm_position_state_[j] = arm_position_command_[j];
      }
    }
    return hardware_interface::return_type::OK;
  }

  if (!serial_.is_open()) return hardware_interface::return_type::ERROR;

  uint8_t buf[256];
  while (true) {
    ssize_t r = serial_.read_bytes(buf, sizeof(buf));
    if (r < 0) return hardware_interface::return_type::ERROR;
    if (r == 0) break;
    parser_.feed(buf, static_cast<size_t>(r));
  }

  while (auto frame = parser_.next_frame()) {
    if (frame->func == protocol::FUNC_REPORT_ENCODER) {
      if (auto counts = protocol::parse_encoder(frame->payload)) {
        update_state_from_encoder_frame(*counts, time);
      }
    } else if (frame->func == protocol::FUNC_REPORT_IMU_ATT) {
      // Roll/pitch/yaw fused by STM32 → quaternion for orientation IF.
      if (auto att = protocol::parse_imu_att(frame->payload)) {
        rpy_to_quat(att->roll, att->pitch, att->yaw,
                    imu_orientation_x_, imu_orientation_y_,
                    imu_orientation_z_, imu_orientation_w_);
      }
    } else if (frame->func == protocol::FUNC_REPORT_ICM_RAW) {
      // ICM20948 raw 9-DOF: gyro (rad/s) → angular_velocity, accel (m/s²)
      // → linear_acceleration. Magnetometer decoded but not exported (no
      // standard broadcaster consumes it from ros2_control state IFs).
      if (auto d = protocol::parse_icm_raw(frame->payload)) {
        imu_angular_velocity_x_    = d->gx;
        imu_angular_velocity_y_    = d->gy;
        imu_angular_velocity_z_    = d->gz;
        imu_linear_acceleration_x_ = d->ax;
        imu_linear_acceleration_y_ = d->ay;
        imu_linear_acceleration_z_ = d->az;
      }
    }
    // FUNC_REPORT_SPEED is drained but unused — vendor-style integrated
    // chassis Twist; mecanum_drive_controller computes its own odom from
    // wheel state, no need to duplicate.
  }

  return hardware_interface::return_type::OK;
}

// Roll/pitch/yaw → unit quaternion (z-y-x intrinsic, ROS REP-103 convention).
// Standard conversion; static so it can be unit-tested without instance.
void YahboomSystem::rpy_to_quat(double roll, double pitch, double yaw,
                                double& qx, double& qy, double& qz, double& qw) {
  const double cr = std::cos(roll  * 0.5);
  const double sr = std::sin(roll  * 0.5);
  const double cp = std::cos(pitch * 0.5);
  const double sp = std::sin(pitch * 0.5);
  const double cy = std::cos(yaw   * 0.5);
  const double sy = std::sin(yaw   * 0.5);
  qw = cr * cp * cy + sr * sp * sy;
  qx = sr * cp * cy - cr * sp * sy;
  qy = cr * sp * cy + sr * cp * sy;
  qz = cr * cp * sy - sr * sp * cy;
}

void YahboomSystem::update_state_from_encoder_frame(
    const protocol::EncoderCounts& counts, const rclcpp::Time& now) {
  const std::array<int32_t, NUM_WHEELS> raw = {counts.m1, counts.m2, counts.m3, counts.m4};

  if (!encoder_seeded_) {
    last_encoder_counts_ = raw;
    last_encoder_time_ = now;
    encoder_seeded_ = true;
    return;  // first frame seeds, no velocity yet
  }

  const double dt = (now - last_encoder_time_).seconds();
  if (dt <= 0.0) return;  // bogus timestamp; skip

  const double rad_per_count = 2.0 * M_PI / encoder_counts_per_rev_;
  // Use kEncoderIndexForWheel mapping — Yahboom physical wiring puts motors 2
  // and 3 swapped vs the naive 1:1 default (verified 2026-05-05 D2.3 calib).
  for (size_t w = 0; w < NUM_WHEELS; ++w) {
    const int e = encoder_index_for_wheel(w);
    const int32_t delta = raw[e] - last_encoder_counts_[e];
    wheel_position_state_[w] += delta * rad_per_count;     // accumulate rad
    wheel_velocity_state_[w]  = (delta * rad_per_count) / dt;
  }
  last_encoder_counts_ = raw;  // store all 4 raw, indexed by encoder slot
  last_encoder_time_ = now;
}

// ─── write() — Path A inverse mecanum kinematics → FUNC_MOTION ───────────────

hardware_interface::return_type YahboomSystem::write(const rclcpp::Time& time,
                                                      const rclcpp::Duration& /*period*/) {
  // Forward mecanum kinematics: per-wheel ω → chassis Twist. Standard formula
  // with wheel-radius R and half-spans Lx (front-rear), Ly (left-right):
  //   vx = (R/4) * ( ωFL + ωFR + ωRL + ωRR)
  //   vy = (R/4) * (-ωFL + ωFR + ωRL - ωRR)
  //   ωz = R / (4 * (Lx+Ly)) * (-ωFL + ωFR - ωRL + ωRR)
  // Same constants as mecanum_drive_controller's INVERSE kinematics → the
  // round-trip cancels and the STM32 receives the operator's original Twist.
  const double R   = wheel_radius_;
  const double Lx  = wheel_separation_x_ / 2.0;
  const double Ly  = wheel_separation_y_ / 2.0;

  const double w_fl = wheel_velocity_command_[FL];
  const double w_fr = wheel_velocity_command_[FR];
  const double w_rl = wheel_velocity_command_[RL];
  const double w_rr = wheel_velocity_command_[RR];

  const double vx = (R / 4.0) * ( w_fl + w_fr + w_rl + w_rr);
  const double vy = (R / 4.0) * (-w_fl + w_fr + w_rl - w_rr);
  const double wz = R / (4.0 * (Lx + Ly)) * (-w_fl + w_fr - w_rl + w_rr);

  // Dedupe + heartbeat. STM32 firmware can't keep up with 100 Hz FUNC_MOTION;
  // each new frame interrupts the previous before it executes. Vendor sends
  // event-driven at ~10-30 Hz. We send only when Twist meaningfully changes
  // OR every kMotionHeartbeatMs to keep any deadman watchdog happy.
  const bool changed =
      !motion_send_seeded_ ||
      std::fabs(vx - last_sent_vx_) > kMotionEpsilon ||
      std::fabs(vy - last_sent_vy_) > kMotionEpsilon ||
      std::fabs(wz - last_sent_wz_) > kMotionEpsilon;

  bool heartbeat = false;
  if (motion_send_seeded_) {
    const auto since_last_ms = (time - last_motion_send_time_).nanoseconds() / 1'000'000;
    heartbeat = since_last_ms >= kMotionHeartbeatMs;
  }

  if (changed || heartbeat) {
    send_motion_command(vx, vy, wz);
    last_sent_vx_ = vx;
    last_sent_vy_ = vy;
    last_sent_wz_ = wz;
    last_motion_send_time_ = time;
    motion_send_seeded_ = true;

    // One-shot debug log on changes to confirm write() is exercised. Throttle
    // by only logging when the command actually changes (not heartbeats).
    if (changed) {
      RCLCPP_INFO(rclcpp::get_logger(kLogger),
                  "FUNC_MOTION send: vx=%.3f vy=%.3f wz=%.3f (from wheel ω: "
                  "FL=%.3f FR=%.3f RL=%.3f RR=%.3f)",
                  vx, vy, wz, w_fl, w_fr, w_rl, w_rr);
    }
  }

  // ── D4: arm path. Convert URDF rad → vendor deg, dedupe + heartbeat,
  // emit FUNC_ARM_CTRL frame. Same throttling pattern as chassis since
  // STM32 firmware likely has the same per-frame buffering issue. In
  // dry_run mode, log only — never touch serial.
  if (arm_present_) {
    std::array<double, NUM_ARM_JOINTS> cmd_deg{};
    for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
      cmd_deg[j] = urdf_rad_to_vendor_deg(j, arm_position_command_[j]);
    }

    bool arm_changed = !arm_send_seeded_;
    if (!arm_changed) {
      for (size_t j = 0; j < NUM_ARM_JOINTS; ++j) {
        if (std::fabs(cmd_deg[j] - last_sent_arm_deg_[j]) > kArmDegEpsilon) {
          arm_changed = true;
          break;
        }
      }
    }
    bool arm_heartbeat = false;
    if (arm_send_seeded_) {
      const auto since_ms = (time - last_arm_send_time_).nanoseconds() / 1'000'000;
      arm_heartbeat = since_ms >= kArmHeartbeatMs;
    }

    if (arm_changed || arm_heartbeat) {
      if (dry_run_) {
        if (arm_changed) {
          RCLCPP_INFO(rclcpp::get_logger(kLogger),
                      "[DRY-RUN] WOULD send FUNC_ARM_CTRL: "
                      "deg=[%.1f %.1f %.1f %.1f %.1f %.1f] "
                      "rad=[%.3f %.3f %.3f %.3f %.3f %.3f]",
                      cmd_deg[0], cmd_deg[1], cmd_deg[2], cmd_deg[3], cmd_deg[4], cmd_deg[5],
                      arm_position_command_[0], arm_position_command_[1],
                      arm_position_command_[2], arm_position_command_[3],
                      arm_position_command_[4], arm_position_command_[5]);
        }
      } else {
        // LIVE mode: build + send the FUNC_ARM_CTRL frame. Only fires once
        // Phase 4 single-joint test passes (operator must set dry_run=false
        // explicitly in URDF).
        std::array<double, 6> angles{
            cmd_deg[0], cmd_deg[1], cmd_deg[2], cmd_deg[3], cmd_deg[4], cmd_deg[5]};
        auto frame = protocol::build_set_uart_servo_angle_array(angles);
        if (serial_.is_open()) {
          serial_.write_bytes(frame.data(), frame.size());
        }
        if (arm_changed) {
          RCLCPP_INFO(rclcpp::get_logger(kLogger),
                      "FUNC_ARM_CTRL send: deg=[%.1f %.1f %.1f %.1f %.1f %.1f]",
                      cmd_deg[0], cmd_deg[1], cmd_deg[2],
                      cmd_deg[3], cmd_deg[4], cmd_deg[5]);
        }
      }
      last_sent_arm_deg_ = cmd_deg;
      last_arm_send_time_ = time;
      arm_send_seeded_ = true;
    }
  }

  return hardware_interface::return_type::OK;
}

void YahboomSystem::send_motion_command(double vx, double vy, double wz) {
  if (dry_run_) {
    RCLCPP_DEBUG(rclcpp::get_logger(kLogger),
                 "DRY RUN: would send FUNC_MOTION vx=%.3f vy=%.3f wz=%.3f",
                 vx, vy, wz);
    return;
  }
  if (!serial_.is_open()) return;
  auto frame = protocol::build_set_car_motion(protocol::CAR_X3_PLUS, vx, vy, wz);
  serial_.write_bytes(frame.data(), frame.size());
}

void YahboomSystem::zero_wheel_command() {
  send_motion_command(0.0, 0.0, 0.0);
}

}  // namespace yahboom_ros2_control

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(yahboom_ros2_control::YahboomSystem,
                       hardware_interface::SystemInterface)
