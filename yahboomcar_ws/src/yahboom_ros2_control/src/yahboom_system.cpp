// D2 chassis-only YahboomSystem implementation. See yahboom_system.hpp for
// architecture notes (Path A = round-trip cancellation via FUNC_MOTION).

#include "yahboom_ros2_control/yahboom_system.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>

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

  // Validate exactly 4 joints declared in URDF, capture their names in order
  // (FL, FR, RL, RR — matches mecanum_drive_controller declaration order).
  if (info_.joints.size() != NUM_WHEELS) {
    RCLCPP_ERROR(rclcpp::get_logger(kLogger),
                 "expected %zu wheel joints in URDF, got %zu",
                 NUM_WHEELS, info_.joints.size());
    return hardware_interface::CallbackReturn::ERROR;
  }

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
                "wheel[%zu] = '%s' → STM32 motor id %d",
                i, wheel_joint_names_[i].c_str(), motor_id_for_wheel(i));
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
  RCLCPP_INFO(rclcpp::get_logger(kLogger), "on_activate OK (wheel commands zeroed)");
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
  out.reserve(2 * NUM_WHEELS);
  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_POSITION,
                     &wheel_position_state_[i]);
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_VELOCITY,
                     &wheel_velocity_state_[i]);
  }
  return out;
}

std::vector<hardware_interface::CommandInterface> YahboomSystem::export_command_interfaces() {
  std::vector<hardware_interface::CommandInterface> out;
  out.reserve(NUM_WHEELS);
  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    out.emplace_back(wheel_joint_names_[i],
                     hardware_interface::HW_IF_VELOCITY,
                     &wheel_velocity_command_[i]);
  }
  return out;
}

// ─── read() — drain push frames, update wheel state ──────────────────────────

hardware_interface::return_type YahboomSystem::read(const rclcpp::Time& time,
                                                     const rclcpp::Duration& /*period*/) {
  if (dry_run_) {
    // Perfect-tracker model: state echoes command.
    for (size_t i = 0; i < NUM_WHEELS; ++i) {
      wheel_velocity_state_[i] = wheel_velocity_command_[i];
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
    }
    // Other push frames (REPORT_IMU_ATT, REPORT_SPEED, REPORT_ICM_RAW) are
    // drained but ignored at D2; D3 wires up imu_sensor_broadcaster.
  }

  return hardware_interface::return_type::OK;
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
  for (size_t i = 0; i < NUM_WHEELS; ++i) {
    const int32_t delta = raw[i] - last_encoder_counts_[i];
    wheel_position_state_[i] += delta * rad_per_count;     // accumulate rad
    wheel_velocity_state_[i]  = (delta * rad_per_count) / dt;
  }
  last_encoder_counts_ = raw;
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
