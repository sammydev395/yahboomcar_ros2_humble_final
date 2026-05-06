// Yahboom ROSMASTER X3PLUS STM32 protocol — port of Rosmaster_Lib v3.3.1
// (~/Repos/yahboomcar_ros2_humble_final/software/py_install_V3.3.1/Rosmaster_Lib).
//
// Wire format:
//   [HEAD=0xFF] [DEVICE_ID=0xFC] [LEN] [FUNC] [payload...] [CHECKSUM]
//
// LEN counts itself + FUNC + payload + checksum, i.e. LEN = N_payload + 3.
// CHECKSUM = (LEN + FUNC + sum(payload)) & 0xff.
// All multi-byte fields are little-endian.
//
// Inbound frames carry head2 = DEVICE_ID - 1 = 0xFB. Important quirk: the
// receiver matches against 0xFB, not 0xFC.
//
// Header-only on purpose. Keeps the protocol layer testable on host without
// linking against rclcpp / hardware_interface.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

namespace yahboom_ros2_control::protocol {

// ─── Wire constants ───────────────────────────────────────────────────────────
constexpr uint8_t HEAD = 0xFF;
constexpr uint8_t DEVICE_ID = 0xFC;
constexpr uint8_t INBOUND_DEVICE_ID = 0xFB;  // DEVICE_ID - 1 — receiver match

// ─── Function codes (subset we actually use) ──────────────────────────────────
enum Func : uint8_t {
  FUNC_AUTO_REPORT = 0x01,
  FUNC_BEEP = 0x02,
  FUNC_RGB = 0x05,

  // Push-mode reports (auto-emitted at ~25-27 Hz when AUTO_REPORT enabled)
  FUNC_REPORT_SPEED = 0x0A,
  FUNC_REPORT_MPU_RAW = 0x0B,    // raw 9-DOF from MPU9250 IMU (older X3PLUS variant)
  FUNC_REPORT_IMU_ATT = 0x0C,    // STM32-fused roll/pitch/yaw
  FUNC_REPORT_ENCODER = 0x0D,
  FUNC_REPORT_ICM_RAW = 0x0E,    // raw 9-DOF from ICM20948 IMU (this X3PLUS unit)

  FUNC_MOTOR = 0x10,           // 4-wheel duty cycle (-100..100)
  FUNC_CAR_RUN = 0x11,         // canned states (forward, etc.) — unused
  FUNC_MOTION = 0x12,          // vx, vy, wz Twist-style
  FUNC_SET_CAR_TYPE = 0x15,

  FUNC_UART_SERVO = 0x20,         // single bus servo, raw pulse
  FUNC_UART_SERVO_TORQUE = 0x22,  // arm torque on/off
  FUNC_ARM_CTRL = 0x23,           // 6-joint batch, raw pulses

  FUNC_REQUEST_DATA = 0x50,       // ask STM32 to emit a data frame on demand;
                                  // payload byte = the FUNC code being requested.
                                  // Used for synchronous reads (e.g. arm
                                  // positions at on_configure / on_activate
                                  // re-seed) — STM32 responds with a frame of
                                  // the requested FUNC type carrying current
                                  // values. Vendor uses this for
                                  // get_uart_servo_angle_array().
};

enum CarType : uint8_t {
  CAR_X3 = 0x01,
  CAR_X3_PLUS = 0x02,
  CAR_X1 = 0x04,
  CAR_R2 = 0x05,
};

// Run-time bound: vendor SDK clamps to 2000 ms. We default to 5 ms to match
// the no-buffered-motion lesson from Ultra Phase 4 (TELEOP_PHASE4_LESSONS.md
// lessons 1-3). Inter-frame interval at 100 Hz is 10 ms; 5 ms gives margin.
constexpr uint16_t DEFAULT_RUN_TIME_MS = 5;

// ─── Checksum ────────────────────────────────────────────────────────────────
// (LEN + FUNC + sum(payload)) & 0xff. Python uses sum(cmd, COMPLEMENT=5)
// which simplifies to this: 5 + HEAD + DEVICE_ID = 5 + 0xFF + 0xFC = 512 ≡ 0
// mod 256, so the COMPLEMENT seed contributes nothing.
inline uint8_t checksum(uint8_t len, uint8_t func, const uint8_t* payload, size_t n) {
  uint16_t sum = static_cast<uint16_t>(len) + static_cast<uint16_t>(func);
  for (size_t i = 0; i < n; ++i) sum += payload[i];
  return static_cast<uint8_t>(sum & 0xff);
}

// ─── Frame builder ───────────────────────────────────────────────────────────
// Build the full wire bytes for a frame: HEAD | DEVICE_ID | LEN | FUNC |
// payload | CHECKSUM. Returns N_payload + 5 bytes.
inline std::vector<uint8_t> build_frame(uint8_t func,
                                        const std::vector<uint8_t>& payload) {
  const uint8_t len = static_cast<uint8_t>(payload.size() + 3);
  std::vector<uint8_t> frame;
  frame.reserve(payload.size() + 5);
  frame.push_back(HEAD);
  frame.push_back(DEVICE_ID);
  frame.push_back(len);
  frame.push_back(func);
  frame.insert(frame.end(), payload.begin(), payload.end());
  frame.push_back(checksum(len, func, payload.data(), payload.size()));
  return frame;
}

// ─── Little-endian helpers ───────────────────────────────────────────────────
inline void push_le16(std::vector<uint8_t>& dst, int16_t v) {
  uint16_t u = static_cast<uint16_t>(v);
  dst.push_back(static_cast<uint8_t>(u & 0xff));
  dst.push_back(static_cast<uint8_t>((u >> 8) & 0xff));
}

inline int16_t read_le16(const uint8_t* src) {
  uint16_t u = static_cast<uint16_t>(src[0]) | (static_cast<uint16_t>(src[1]) << 8);
  return static_cast<int16_t>(u);
}

inline int32_t read_le32(const uint8_t* src) {
  uint32_t u = static_cast<uint32_t>(src[0]) |
               (static_cast<uint32_t>(src[1]) << 8) |
               (static_cast<uint32_t>(src[2]) << 16) |
               (static_cast<uint32_t>(src[3]) << 24);
  return static_cast<int32_t>(u);
}

// ─── Arm angle ↔ pulse conversion ────────────────────────────────────────────
// Per Rosmaster_Lib::__arm_convert_value. Asymmetric across joints:
//   joints 1-4: 0..180° → 3100..900 pulse (INVERTED)
//   joint   5: 0..270° → 380..3700 pulse
//   joint   6: 0..180° → 900..3100 pulse
// `s_id` is 1-indexed (1..6) to match vendor convention.
inline int16_t arm_angle_to_pulse(uint8_t s_id, double angle_deg) {
  switch (s_id) {
    case 1: case 2: case 3: case 4:
      return static_cast<int16_t>((3100.0 - 900.0) * (angle_deg - 180.0) / (0.0 - 180.0) + 900.0);
    case 5:
      return static_cast<int16_t>((3700.0 - 380.0) * (angle_deg - 0.0) / (270.0 - 0.0) + 380.0);
    case 6:
      return static_cast<int16_t>((3100.0 - 900.0) * (angle_deg - 0.0) / (180.0 - 0.0) + 900.0);
    default:
      return -1;
  }
}

inline double arm_pulse_to_angle(uint8_t s_id, int16_t pulse) {
  const double p = static_cast<double>(pulse);
  switch (s_id) {
    case 1: case 2: case 3: case 4:
      return (p - 900.0) * (0.0 - 180.0) / (3100.0 - 900.0) + 180.0;
    case 5:
      return (270.0 - 0.0) * (p - 380.0) / (3700.0 - 380.0) + 0.0;
    case 6:
      return (180.0 - 0.0) * (p - 900.0) / (3100.0 - 900.0) + 0.0;
    default:
      return std::nan("");
  }
}

inline int8_t clamp_motor_duty(int v) {
  if (v > 100) return 100;
  if (v < -100) return -100;
  return static_cast<int8_t>(v);
}

// ─── Outbound frame builders ─────────────────────────────────────────────────

// FUNC_MOTOR: per-wheel PWM duty cycle, range [-100, 100].
inline std::vector<uint8_t> build_set_motor(int s1, int s2, int s3, int s4) {
  std::vector<uint8_t> payload;
  payload.push_back(static_cast<uint8_t>(clamp_motor_duty(s1)));
  payload.push_back(static_cast<uint8_t>(clamp_motor_duty(s2)));
  payload.push_back(static_cast<uint8_t>(clamp_motor_duty(s3)));
  payload.push_back(static_cast<uint8_t>(clamp_motor_duty(s4)));
  return build_frame(FUNC_MOTOR, payload);
}

// FUNC_MOTION: Twist-style chassis command. Vendor multiplies vx, vy, wz by
// 1000 and packs as int16. X3PLUS valid range: vx, vy ∈ [-0.7, +0.7] m/s,
// wz ∈ [-3.2, +3.2] rad/s.
inline std::vector<uint8_t> build_set_car_motion(CarType car_type,
                                                 double vx, double vy, double wz) {
  std::vector<uint8_t> payload;
  payload.push_back(static_cast<uint8_t>(car_type));
  push_le16(payload, static_cast<int16_t>(vx * 1000.0));
  push_le16(payload, static_cast<int16_t>(vy * 1000.0));
  push_le16(payload, static_cast<int16_t>(wz * 1000.0));
  return build_frame(FUNC_MOTION, payload);
}

// FUNC_UART_SERVO: single bus servo, raw pulse [96, 4000], run_time [0, 2000] ms.
inline std::vector<uint8_t> build_set_uart_servo(uint8_t servo_id, int16_t pulse,
                                                 uint16_t run_time_ms = DEFAULT_RUN_TIME_MS) {
  if (run_time_ms > 2000) run_time_ms = 2000;
  std::vector<uint8_t> payload;
  payload.push_back(servo_id);
  push_le16(payload, pulse);
  push_le16(payload, static_cast<int16_t>(run_time_ms));
  return build_frame(FUNC_UART_SERVO, payload);
}

inline std::vector<uint8_t> build_set_uart_servo_angle(uint8_t servo_id, double angle_deg,
                                                       uint16_t run_time_ms = DEFAULT_RUN_TIME_MS) {
  return build_set_uart_servo(servo_id, arm_angle_to_pulse(servo_id, angle_deg), run_time_ms);
}

// FUNC_ARM_CTRL: batch all 6 servos in one frame. Angles are deg; vendor
// validates s1-s4 ∈ [0,180], s5 ∈ [0,270], s6 ∈ [0,180].
inline std::vector<uint8_t> build_set_uart_servo_angle_array(
    const std::array<double, 6>& angles_deg,
    uint16_t run_time_ms = DEFAULT_RUN_TIME_MS) {
  if (run_time_ms > 2000) run_time_ms = 2000;
  std::vector<uint8_t> payload;
  for (size_t i = 0; i < 6; ++i) {
    push_le16(payload, arm_angle_to_pulse(static_cast<uint8_t>(i + 1), angles_deg[i]));
  }
  push_le16(payload, static_cast<int16_t>(run_time_ms));
  return build_frame(FUNC_ARM_CTRL, payload);
}

// FUNC_UART_SERVO_TORQUE: 0 = limp (manually movable), 1 = enabled.
// NOTE: vendor hardcodes LEN=0x04 here (payload = 1 byte), which matches the
// computed N+3 = 4. We preserve the auto-computation.
inline std::vector<uint8_t> build_set_uart_servo_torque(bool enable) {
  std::vector<uint8_t> payload;
  payload.push_back(enable ? 0x01 : 0x00);
  return build_frame(FUNC_UART_SERVO_TORQUE, payload);
}

// FUNC_AUTO_REPORT: enable push-mode encoder + IMU + speed reports. forever
// = true persists to flash (slow), false is RAM-only (fast). state byte
// layout: state1 = enable (0/1), state2 = forever (0/1).
inline std::vector<uint8_t> build_set_auto_report_state(bool enable, bool forever) {
  std::vector<uint8_t> payload;
  payload.push_back(enable ? 0x01 : 0x00);
  payload.push_back(forever ? 0x01 : 0x00);
  return build_frame(FUNC_AUTO_REPORT, payload);
}

inline std::vector<uint8_t> build_set_car_type(CarType car_type) {
  std::vector<uint8_t> payload;
  payload.push_back(static_cast<uint8_t>(car_type));
  payload.push_back(0x5F);  // forever flag, vendor hardcodes
  return build_frame(FUNC_SET_CAR_TYPE, payload);
}

// FUNC_REQUEST_DATA: ask STM32 to emit a one-shot frame of `target_func`
// type. Used for synchronous reads — e.g. requesting FUNC_ARM_CTRL gets
// a 6-int16-pulse arm-positions response back via the same parser stream.
// Vendor's Rosmaster_Lib.__request_data wraps this; param is unused for
// FUNC_ARM_CTRL queries (vendor passes 0).
inline std::vector<uint8_t> build_request_data(uint8_t target_func, uint8_t param = 0) {
  std::vector<uint8_t> payload;
  payload.push_back(target_func);
  payload.push_back(param);
  return build_frame(FUNC_REQUEST_DATA, payload);
}

// ─── Inbound frame parsing ───────────────────────────────────────────────────

struct ParsedFrame {
  uint8_t func;
  std::vector<uint8_t> payload;
};

// Streaming parser. Feed bytes one at a time; call next_frame() to drain
// completed frames. Bad checksums are dropped silently (matching vendor
// behavior); the parser resyncs on the next HEAD byte.
class FrameParser {
 public:
  void feed(uint8_t b) {
    switch (state_) {
      case State::WaitHead:
        if (b == HEAD) state_ = State::WaitHead2;
        break;
      case State::WaitHead2:
        if (b == INBOUND_DEVICE_ID) {
          state_ = State::WaitLen;
        } else if (b == HEAD) {
          // double HEAD — stay
        } else {
          state_ = State::WaitHead;
        }
        break;
      case State::WaitLen:
        len_ = b;
        state_ = State::WaitFunc;
        break;
      case State::WaitFunc:
        func_ = b;
        payload_.clear();
        // data_len = LEN - 2: how many bytes follow FUNC (= payload + checksum)
        if (len_ < 2) {
          state_ = State::WaitHead;  // malformed, resync
          break;
        }
        bytes_remaining_ = static_cast<size_t>(len_ - 2);
        if (bytes_remaining_ == 0) {
          // pathological: no payload, no checksum slot
          state_ = State::WaitHead;
          break;
        }
        state_ = State::ReadPayload;
        break;
      case State::ReadPayload:
        if (bytes_remaining_ > 1) {
          payload_.push_back(b);
          --bytes_remaining_;
        } else {
          // Last byte is the checksum.
          const uint8_t expected = checksum(len_, func_, payload_.data(), payload_.size());
          if (b == expected) {
            ready_.push_back(ParsedFrame{func_, std::move(payload_)});
          }
          payload_.clear();
          state_ = State::WaitHead;
        }
        break;
    }
  }

  void feed(const uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) feed(buf[i]);
  }

  std::optional<ParsedFrame> next_frame() {
    if (ready_.empty()) return std::nullopt;
    ParsedFrame f = std::move(ready_.front());
    ready_.erase(ready_.begin());
    return f;
  }

  void reset() {
    state_ = State::WaitHead;
    payload_.clear();
    ready_.clear();
  }

 private:
  enum class State { WaitHead, WaitHead2, WaitLen, WaitFunc, ReadPayload };
  State state_ = State::WaitHead;
  uint8_t len_ = 0;
  uint8_t func_ = 0;
  size_t bytes_remaining_ = 0;
  std::vector<uint8_t> payload_;
  std::vector<ParsedFrame> ready_;
};

// ─── Typed accessors for push-mode payloads ──────────────────────────────────
// All angles in radians (vendor units * (1/10000) for IMU att).

struct ImuAttitude {
  double roll;   // rad
  double pitch;  // rad
  double yaw;    // rad
};

inline std::optional<ImuAttitude> parse_imu_att(const std::vector<uint8_t>& payload) {
  if (payload.size() < 6) return std::nullopt;
  return ImuAttitude{
      read_le16(payload.data() + 0) / 10000.0,
      read_le16(payload.data() + 2) / 10000.0,
      read_le16(payload.data() + 4) / 10000.0,
  };
}

struct EncoderCounts {
  int32_t m1, m2, m3, m4;
};

inline std::optional<EncoderCounts> parse_encoder(const std::vector<uint8_t>& payload) {
  if (payload.size() < 16) return std::nullopt;
  return EncoderCounts{
      read_le32(payload.data() + 0),
      read_le32(payload.data() + 4),
      read_le32(payload.data() + 8),
      read_le32(payload.data() + 12),
  };
}

struct SpeedReport {
  double vx;            // m/s
  double vy;            // m/s
  double wz;            // rad/s
  uint8_t battery_v10;  // battery voltage * 10 (e.g., 121 → 12.1 V)
};

inline std::optional<SpeedReport> parse_speed(const std::vector<uint8_t>& payload) {
  if (payload.size() < 7) return std::nullopt;
  return SpeedReport{
      read_le16(payload.data() + 0) / 1000.0,
      read_le16(payload.data() + 2) / 1000.0,
      read_le16(payload.data() + 4) / 1000.0,
      payload[6],
  };
}

// Raw 9-DOF IMU readings from ICM20948 (FUNC_REPORT_ICM_RAW). Per
// Rosmaster_Lib v3.3.1 line ~166-182, all 9 fields are int16 LE divided by 1000:
//   gyro: rad/s (verified — stationary robot reports ≈ 0.001-0.003)
//   accel: m/s² (verified — accel_z ≈ -9.96 = -1g pointing down)
//   mag: µT-ish (Earth field at ≈ 22 µT magnitude — chip-default scaling)
// Vendor MPU_RAW (0x0B) uses different per-axis ratios + sign flips on gy/gz;
// not implemented here because this X3PLUS unit reports ICM_RAW only.
struct IcmRawData {
  double gx, gy, gz;  // gyroscope (rad/s)
  double ax, ay, az;  // accelerometer (m/s²)
  double mx, my, mz;  // magnetometer (µT)
};

inline std::optional<IcmRawData> parse_icm_raw(const std::vector<uint8_t>& payload) {
  if (payload.size() < 18) return std::nullopt;
  return IcmRawData{
      read_le16(payload.data() + 0)  / 1000.0,
      read_le16(payload.data() + 2)  / 1000.0,
      read_le16(payload.data() + 4)  / 1000.0,
      read_le16(payload.data() + 6)  / 1000.0,
      read_le16(payload.data() + 8)  / 1000.0,
      read_le16(payload.data() + 10) / 1000.0,
      read_le16(payload.data() + 12) / 1000.0,
      read_le16(payload.data() + 14) / 1000.0,
      read_le16(payload.data() + 16) / 1000.0,
  };
}

// FUNC_ARM_CTRL response (one-shot, in reply to FUNC_REQUEST_DATA(FUNC_ARM_CTRL)):
// 6 int16 LE pulse values, one per servo (s1..s6 in vendor order).
// Caller converts pulse → vendor degrees with arm_pulse_to_angle(s_id, pulse),
// then to URDF rad via per-joint offset/sign at the YahboomSystem layer.
struct ArmPulses {
  std::array<int16_t, 6> pulses;  // [s1, s2, s3, s4, s5, s6]
};

inline std::optional<ArmPulses> parse_arm_ctrl(const std::vector<uint8_t>& payload) {
  if (payload.size() < 12) return std::nullopt;
  ArmPulses out;
  for (size_t i = 0; i < 6; ++i) {
    out.pulses[i] = read_le16(payload.data() + 2 * i);
  }
  return out;
}

}  // namespace yahboom_ros2_control::protocol
