// Unit tests for yahboom_ros2_control::protocol — pure host tests, no
// hardware. Expected wire bytes hand-computed from the Rosmaster_Lib v3.3.1
// reference at ~/Repos/yahboomcar_ros2_humble_final/software/py_install_V3.3.1/.

#include <gtest/gtest.h>

#include "yahboom_ros2_control/yahboom_protocol.hpp"

namespace y = yahboom_ros2_control::protocol;

// ─── Checksum ────────────────────────────────────────────────────────────────

TEST(Checksum, ZeroPayload) {
  // FUNC_MOTOR with all-zero payload: checksum = (LEN=7 + FUNC=0x10) & 0xff = 0x17
  EXPECT_EQ(y::checksum(7, 0x10, nullptr, 0), 0x17);
}

TEST(Checksum, MotorWithSignedBytes) {
  // set_motor(50, -50, 50, -50): payload bytes are 50, 0xCE, 50, 0xCE.
  uint8_t payload[] = {50, 0xCE, 50, 0xCE};
  // (7 + 16 + 50 + 206 + 50 + 206) = 535 → & 0xff = 23 = 0x17
  EXPECT_EQ(y::checksum(7, y::FUNC_MOTOR, payload, 4), 0x17);
}

TEST(Checksum, OverflowWrap) {
  // Construct a payload that forces multi-wrap.
  uint8_t payload[] = {0xFF, 0xFF, 0xFF, 0xFF};
  // 7 + 16 + 4*255 = 1043 → & 0xff = 19 = 0x13
  EXPECT_EQ(y::checksum(7, y::FUNC_MOTOR, payload, 4), 0x13);
}

// ─── Frame builder ───────────────────────────────────────────────────────────

TEST(BuildFrame, SetMotorAllZero) {
  auto f = y::build_set_motor(0, 0, 0, 0);
  ASSERT_EQ(f.size(), 9u);
  EXPECT_EQ(f[0], 0xFF);  // HEAD
  EXPECT_EQ(f[1], 0xFC);  // DEVICE_ID
  EXPECT_EQ(f[2], 0x07);  // LEN = N+3 = 4+3
  EXPECT_EQ(f[3], 0x10);  // FUNC_MOTOR
  EXPECT_EQ(f[4], 0x00);
  EXPECT_EQ(f[5], 0x00);
  EXPECT_EQ(f[6], 0x00);
  EXPECT_EQ(f[7], 0x00);
  EXPECT_EQ(f[8], 0x17);  // checksum
}

TEST(BuildFrame, SetMotorMixedSign) {
  auto f = y::build_set_motor(50, -50, 50, -50);
  ASSERT_EQ(f.size(), 9u);
  EXPECT_EQ(f[4], 50);
  EXPECT_EQ(f[5], 0xCE);
  EXPECT_EQ(f[6], 50);
  EXPECT_EQ(f[7], 0xCE);
  EXPECT_EQ(f[8], 0x17);
}

TEST(BuildFrame, SetMotorClampsOver100) {
  auto f = y::build_set_motor(200, -200, 50, 0);
  EXPECT_EQ(f[4], 100);
  EXPECT_EQ(static_cast<int8_t>(f[5]), -100);
  EXPECT_EQ(f[6], 50);
  EXPECT_EQ(f[7], 0);
}

TEST(BuildFrame, SetCarMotionLittleEndian) {
  // X3_PLUS, vx=0.5, vy=0.0, wz=1.0 → vx*1000=500=0x01F4, wz*1000=1000=0x03E8
  auto f = y::build_set_car_motion(y::CAR_X3_PLUS, 0.5, 0.0, 1.0);
  ASSERT_EQ(f.size(), 12u);
  EXPECT_EQ(f[0], 0xFF);
  EXPECT_EQ(f[1], 0xFC);
  EXPECT_EQ(f[2], 0x0A);  // LEN = 7+3
  EXPECT_EQ(f[3], 0x12);  // FUNC_MOTION
  EXPECT_EQ(f[4], 0x02);  // CAR_X3_PLUS
  EXPECT_EQ(f[5], 0xF4);  // vx low
  EXPECT_EQ(f[6], 0x01);  // vx high
  EXPECT_EQ(f[7], 0x00);  // vy low
  EXPECT_EQ(f[8], 0x00);  // vy high
  EXPECT_EQ(f[9], 0xE8);  // wz low
  EXPECT_EQ(f[10], 0x03); // wz high
  EXPECT_EQ(f[11], 0xFE); // checksum: (10+18+2+244+1+0+0+232+3) & 0xff = 510 & 0xff = 254
}

TEST(BuildFrame, SetCarMotionNegativeIsLittleEndianSigned) {
  // vx = -0.5 → -500 → 0xFE0C little-endian (-500 = 0xFE0C as int16)
  auto f = y::build_set_car_motion(y::CAR_X3_PLUS, -0.5, 0.0, 0.0);
  EXPECT_EQ(f[5], 0x0C);  // -500 low byte
  EXPECT_EQ(f[6], 0xFE);  // -500 high byte (sign extension)
}

TEST(BuildFrame, SetUartServoTorqueOn) {
  auto f = y::build_set_uart_servo_torque(true);
  ASSERT_EQ(f.size(), 6u);
  EXPECT_EQ(f[0], 0xFF);
  EXPECT_EQ(f[1], 0xFC);
  EXPECT_EQ(f[2], 0x04);  // LEN = 1+3
  EXPECT_EQ(f[3], 0x22);  // FUNC_UART_SERVO_TORQUE
  EXPECT_EQ(f[4], 0x01);
  EXPECT_EQ(f[5], 0x27);  // (4+0x22+1) & 0xff = 39 = 0x27
}

TEST(BuildFrame, SetUartServoTorqueOff) {
  auto f = y::build_set_uart_servo_torque(false);
  EXPECT_EQ(f[4], 0x00);
  EXPECT_EQ(f[5], 0x26);
}

TEST(BuildFrame, SetUartServoUsesShortRunTimeByDefault) {
  // DEFAULT_RUN_TIME_MS = 500, matching vendor's Rosmaster_Lib exactly.
  // (Initial 5 ms produced oscillation. 100 ms fixed FUNC_ARM_CTRL but
  // not FUNC_UART_SERVO — single-servo frames were silently ignored
  // until run_time hit ≥ 500 ms, verified live D7.5 2026-05-07 via
  // smoke_serial --move-servo. Same value works for both frame types
  // and matches the vendor SDK we ported from.) Test name kept as-is
  // for git history continuity even though "Short" is now misleading.
  auto f = y::build_set_uart_servo(1, 2000);
  ASSERT_EQ(f.size(), 10u);
  EXPECT_EQ(f[3], 0x20);  // FUNC_UART_SERVO
  EXPECT_EQ(f[4], 1);     // servo_id
  EXPECT_EQ(f[5], 0xD0);  // pulse=2000=0x07D0 LE
  EXPECT_EQ(f[6], 0x07);
  EXPECT_EQ(f[7], 0xF4);  // run_time=500=0x01F4 LE
  EXPECT_EQ(f[8], 0x01);
}

TEST(BuildFrame, SetUartServoClampsRunTime) {
  // Vendor SDK clamps run_time to 2000 ms.
  auto f = y::build_set_uart_servo(1, 1000, 5000);
  EXPECT_EQ(f[7], 0xD0);  // 2000=0x07D0
  EXPECT_EQ(f[8], 0x07);
}

TEST(BuildFrame, AutoReportEnable) {
  auto f = y::build_set_auto_report_state(true, false);
  ASSERT_EQ(f.size(), 7u);
  EXPECT_EQ(f[2], 0x05);  // LEN = 2+3
  EXPECT_EQ(f[3], 0x01);  // FUNC_AUTO_REPORT
  EXPECT_EQ(f[4], 0x01);  // enable
  EXPECT_EQ(f[5], 0x00);  // not forever
}

// ─── Arm angle ↔ pulse conversion ────────────────────────────────────────────

TEST(ArmConversion, Joint1To4HomePoses) {
  // Joints 1-4: 0..180° → 3100..900 (INVERTED). Memory says X3PLUS home is
  // [90, 145, 0, 45, 90, 30] degrees for [j1, j2, j3, j4, j5, j6].
  EXPECT_EQ(y::arm_angle_to_pulse(1, 90.0), 2000);   // mid
  EXPECT_EQ(y::arm_angle_to_pulse(2, 145.0), 1327);  // shoulder home
  EXPECT_EQ(y::arm_angle_to_pulse(3, 0.0), 3100);    // elbow home (max pulse)
  EXPECT_EQ(y::arm_angle_to_pulse(4, 45.0), 2550);   // wrist home
}

TEST(ArmConversion, Joint5IsForwardNot270) {
  // Joint 5: 0..270° → 380..3700 (FORWARD)
  EXPECT_EQ(y::arm_angle_to_pulse(5, 0.0), 380);
  EXPECT_EQ(y::arm_angle_to_pulse(5, 90.0), 1486);   // home, ≈ 380 + (3320/3)
  EXPECT_EQ(y::arm_angle_to_pulse(5, 270.0), 3700);
}

TEST(ArmConversion, Joint6GripperIsForward) {
  // Joint 6: 0..180° → 900..3100 (FORWARD). Gripper: 30°=closed, 180°=open.
  EXPECT_EQ(y::arm_angle_to_pulse(6, 0.0), 900);
  EXPECT_EQ(y::arm_angle_to_pulse(6, 30.0), 1266);   // closed
  EXPECT_EQ(y::arm_angle_to_pulse(6, 180.0), 3100);  // open
}

TEST(ArmConversion, RoundTrip) {
  // Round-trip every joint at its home angle. Vendor uses int truncation, so
  // accept ±1 pulse drift.
  const double home_angles[6] = {90.0, 145.0, 0.0, 45.0, 90.0, 30.0};
  for (int i = 0; i < 6; ++i) {
    const uint8_t s_id = static_cast<uint8_t>(i + 1);
    int16_t p = y::arm_angle_to_pulse(s_id, home_angles[i]);
    double back = y::arm_pulse_to_angle(s_id, p);
    EXPECT_NEAR(back, home_angles[i], 0.5)
        << "joint=" << static_cast<int>(s_id) << " pulse=" << p;
  }
}

// ─── Little-endian helpers ───────────────────────────────────────────────────

TEST(LittleEndian, PushAndReadInt16) {
  std::vector<uint8_t> buf;
  y::push_le16(buf, -500);
  ASSERT_EQ(buf.size(), 2u);
  EXPECT_EQ(buf[0], 0x0C);
  EXPECT_EQ(buf[1], 0xFE);
  EXPECT_EQ(y::read_le16(buf.data()), -500);
}

TEST(LittleEndian, ReadInt32) {
  uint8_t buf[] = {0x39, 0x30, 0x00, 0x00};
  EXPECT_EQ(y::read_le32(buf), 12345);
}

TEST(LittleEndian, ReadInt32Negative) {
  // -12345 = 0xFFFFCFC7 little-endian = {0xC7, 0xCF, 0xFF, 0xFF}
  uint8_t buf[] = {0xC7, 0xCF, 0xFF, 0xFF};
  EXPECT_EQ(y::read_le32(buf), -12345);
}

// ─── Inbound frame parser ────────────────────────────────────────────────────

static std::vector<uint8_t> make_inbound_frame(uint8_t func,
                                               const std::vector<uint8_t>& payload) {
  // Inbound frames carry head2 = INBOUND_DEVICE_ID (0xFB), but use the same
  // LEN/CHECKSUM math as outbound. Cf. Rosmaster_Lib::__receive_data.
  const uint8_t len = static_cast<uint8_t>(payload.size() + 3);
  std::vector<uint8_t> bytes;
  bytes.push_back(y::HEAD);
  bytes.push_back(y::INBOUND_DEVICE_ID);
  bytes.push_back(len);
  bytes.push_back(func);
  bytes.insert(bytes.end(), payload.begin(), payload.end());
  bytes.push_back(y::checksum(len, func, payload.data(), payload.size()));
  return bytes;
}

TEST(Parser, EmitsCompleteFrame) {
  std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04};
  auto wire = make_inbound_frame(y::FUNC_REPORT_SPEED, payload);
  y::FrameParser p;
  p.feed(wire.data(), wire.size());
  auto f = p.next_frame();
  ASSERT_TRUE(f.has_value());
  EXPECT_EQ(f->func, y::FUNC_REPORT_SPEED);
  EXPECT_EQ(f->payload, payload);
  EXPECT_FALSE(p.next_frame().has_value());
}

TEST(Parser, DropsBadChecksum) {
  std::vector<uint8_t> payload = {0x01, 0x02};
  auto wire = make_inbound_frame(y::FUNC_REPORT_IMU_ATT, payload);
  wire.back() ^= 0xFF;  // corrupt checksum
  y::FrameParser p;
  p.feed(wire.data(), wire.size());
  EXPECT_FALSE(p.next_frame().has_value());
}

TEST(Parser, ResyncsOnHeadInGarbage) {
  std::vector<uint8_t> garbage = {0x42, 0x99, 0xAB};
  std::vector<uint8_t> payload = {0xAA};
  auto good = make_inbound_frame(y::FUNC_REPORT_SPEED, payload);
  std::vector<uint8_t> stream;
  stream.insert(stream.end(), garbage.begin(), garbage.end());
  stream.insert(stream.end(), good.begin(), good.end());
  y::FrameParser p;
  p.feed(stream.data(), stream.size());
  auto f = p.next_frame();
  ASSERT_TRUE(f.has_value());
  EXPECT_EQ(f->func, y::FUNC_REPORT_SPEED);
  EXPECT_EQ(f->payload, payload);
}

TEST(Parser, RejectsOutboundDeviceId) {
  // Outbound DEVICE_ID = 0xFC must NOT be accepted as an inbound frame —
  // receiver matches against 0xFB only.
  std::vector<uint8_t> bytes = {y::HEAD, 0xFC, 0x05, 0x0A, 0x01, 0x02, 0x18};
  y::FrameParser p;
  p.feed(bytes.data(), bytes.size());
  EXPECT_FALSE(p.next_frame().has_value());
}

TEST(Parser, HandlesByteAtATimeStreaming) {
  std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
  auto wire = make_inbound_frame(y::FUNC_REPORT_ENCODER, payload);
  y::FrameParser p;
  for (uint8_t b : wire) {
    p.feed(b);
  }
  auto f = p.next_frame();
  ASSERT_TRUE(f.has_value());
  EXPECT_EQ(f->payload, payload);
}

TEST(Parser, MultipleFramesBackToBack) {
  std::vector<uint8_t> stream;
  for (int i = 0; i < 3; ++i) {
    auto w = make_inbound_frame(y::FUNC_REPORT_SPEED, {static_cast<uint8_t>(i)});
    stream.insert(stream.end(), w.begin(), w.end());
  }
  y::FrameParser p;
  p.feed(stream.data(), stream.size());
  for (int i = 0; i < 3; ++i) {
    auto f = p.next_frame();
    ASSERT_TRUE(f.has_value()) << "frame " << i;
    EXPECT_EQ(f->payload[0], static_cast<uint8_t>(i));
  }
  EXPECT_FALSE(p.next_frame().has_value());
}

// ─── Typed push-mode parsers ─────────────────────────────────────────────────

TEST(TypedParse, ImuAttitude) {
  // roll = 0.5 rad → int16(5000) = 0x1388 LE = {0x88, 0x13}
  // pitch = -0.25 → int16(-2500) = 0xF63C LE = {0x3C, 0xF6}
  // yaw = 1.5707 → int16(15707) = 0x3D5B LE = {0x5B, 0x3D}
  std::vector<uint8_t> payload = {0x88, 0x13, 0x3C, 0xF6, 0x5B, 0x3D};
  auto a = y::parse_imu_att(payload);
  ASSERT_TRUE(a.has_value());
  EXPECT_NEAR(a->roll, 0.5, 1e-4);
  EXPECT_NEAR(a->pitch, -0.25, 1e-4);
  EXPECT_NEAR(a->yaw, 1.5707, 1e-4);
}

TEST(TypedParse, EncoderCounts) {
  // m1=12345, m2=-12345, m3=0, m4=2147483647 (INT32_MAX)
  std::vector<uint8_t> payload = {
      0x39, 0x30, 0x00, 0x00,  // 12345
      0xC7, 0xCF, 0xFF, 0xFF,  // -12345
      0x00, 0x00, 0x00, 0x00,  // 0
      0xFF, 0xFF, 0xFF, 0x7F,  // INT32_MAX
  };
  auto e = y::parse_encoder(payload);
  ASSERT_TRUE(e.has_value());
  EXPECT_EQ(e->m1, 12345);
  EXPECT_EQ(e->m2, -12345);
  EXPECT_EQ(e->m3, 0);
  EXPECT_EQ(e->m4, 2147483647);
}

TEST(TypedParse, SpeedReport) {
  // vx=0.5, vy=0.0, wz=-1.0, battery=121 (12.1 V)
  std::vector<uint8_t> payload = {
      0xF4, 0x01,  // 500
      0x00, 0x00,  // 0
      0x18, 0xFC,  // -1000
      0x79,        // 121
  };
  auto s = y::parse_speed(payload);
  ASSERT_TRUE(s.has_value());
  EXPECT_NEAR(s->vx, 0.5, 1e-6);
  EXPECT_NEAR(s->vy, 0.0, 1e-6);
  EXPECT_NEAR(s->wz, -1.0, 1e-6);
  EXPECT_EQ(s->battery_v10, 121);
}

TEST(TypedParse, RejectsTruncatedPayload) {
  std::vector<uint8_t> short_payload = {0x88, 0x13};  // only 2 bytes
  EXPECT_FALSE(y::parse_imu_att(short_payload).has_value());
  EXPECT_FALSE(y::parse_encoder(short_payload).has_value());
  EXPECT_FALSE(y::parse_speed(short_payload).has_value());
  EXPECT_FALSE(y::parse_icm_raw(short_payload).has_value());
}

TEST(TypedParse, IcmRawFromLiveJetsonCapture) {
  // Captured 2026-05-04 from /dev/myserial smoke test on jetsonnanodev:
  //   FUNC=0x0E payload[18]=02 00 FE FF 00 00 DD FF DD FF 17 D9 0A AB 2F A5 4C 49
  // Decoded as 9× int16 LE divided by 1000:
  //   gx=2/1000=0.002, gy=-2/1000=-0.002, gz=0
  //   ax=-35/1000=-0.035, ay=-35/1000=-0.035, az=-9961/1000=-9.961 (≈ -1g, gravity ✓)
  //   mx=-21750/1000=-21.75, my=-23249/1000=-23.249, mz=18764/1000=18.764
  std::vector<uint8_t> payload = {
      0x02, 0x00, 0xFE, 0xFF, 0x00, 0x00,  // gyro
      0xDD, 0xFF, 0xDD, 0xFF, 0x17, 0xD9,  // accel
      0x0A, 0xAB, 0x2F, 0xA5, 0x4C, 0x49,  // mag
  };
  auto d = y::parse_icm_raw(payload);
  ASSERT_TRUE(d.has_value());
  EXPECT_NEAR(d->gx,  0.002, 1e-4);
  EXPECT_NEAR(d->gy, -0.002, 1e-4);
  EXPECT_NEAR(d->gz,  0.000, 1e-4);
  EXPECT_NEAR(d->ax, -0.035, 1e-4);
  EXPECT_NEAR(d->ay, -0.035, 1e-4);
  EXPECT_NEAR(d->az, -9.961, 1e-3);  // gravity sanity check — 1g ≈ 9.81 m/s²
  EXPECT_NEAR(d->mx, -21.750, 1e-3);
  EXPECT_NEAR(d->my, -23.249, 1e-3);
  EXPECT_NEAR(d->mz,  18.764, 1e-3);
}

// ─── FUNC_REQUEST_DATA + FUNC_ARM_CTRL response (D7) ─────────────────────────

TEST(BuildFrame, RequestDataForArmCtrl) {
  // build_request_data(FUNC_ARM_CTRL) — payload = [0x23, 0x00], LEN = 2+3 = 5.
  // checksum = (5 + 0x50 + 0x23 + 0x00) & 0xff = 0x78
  auto f = y::build_request_data(y::FUNC_ARM_CTRL);
  ASSERT_EQ(f.size(), 7u);
  EXPECT_EQ(f[0], 0xFF);
  EXPECT_EQ(f[1], 0xFC);
  EXPECT_EQ(f[2], 0x05);  // LEN
  EXPECT_EQ(f[3], 0x50);  // FUNC_REQUEST_DATA
  EXPECT_EQ(f[4], 0x23);  // target = FUNC_ARM_CTRL
  EXPECT_EQ(f[5], 0x00);  // param
  EXPECT_EQ(f[6], 0x78);  // checksum
}

TEST(TypedParse, ArmCtrlSixPulses) {
  // 6 × int16 LE pulses. Vendor home pose pulses: s1=2000, s2=1327, s3=3100,
  // s4=2550, s5=1486, s6=1266 (computed via arm_angle_to_pulse for the
  // documented degrees [90, 145, 0, 45, 90, 30]).
  auto le = [](int16_t v) -> std::array<uint8_t, 2> {
    return {static_cast<uint8_t>(v & 0xff),
            static_cast<uint8_t>((v >> 8) & 0xff)};
  };
  std::vector<uint8_t> payload;
  for (int16_t p : {int16_t(2000), int16_t(1327), int16_t(3100),
                    int16_t(2550), int16_t(1486), int16_t(1266)}) {
    auto bytes = le(p);
    payload.push_back(bytes[0]);
    payload.push_back(bytes[1]);
  }
  auto a = y::parse_arm_ctrl(payload);
  ASSERT_TRUE(a.has_value());
  EXPECT_EQ(a->pulses[0], 2000);
  EXPECT_EQ(a->pulses[1], 1327);
  EXPECT_EQ(a->pulses[2], 3100);
  EXPECT_EQ(a->pulses[3], 2550);
  EXPECT_EQ(a->pulses[4], 1486);
  EXPECT_EQ(a->pulses[5], 1266);
  // Round-trip via arm_pulse_to_angle: should recover home degrees ±0.5°.
  EXPECT_NEAR(y::arm_pulse_to_angle(1, a->pulses[0]),  90.0, 0.5);
  EXPECT_NEAR(y::arm_pulse_to_angle(2, a->pulses[1]), 145.0, 0.5);
  EXPECT_NEAR(y::arm_pulse_to_angle(3, a->pulses[2]),   0.0, 0.5);
  EXPECT_NEAR(y::arm_pulse_to_angle(4, a->pulses[3]),  45.0, 0.5);
  EXPECT_NEAR(y::arm_pulse_to_angle(5, a->pulses[4]),  90.0, 0.5);
  EXPECT_NEAR(y::arm_pulse_to_angle(6, a->pulses[5]),  30.0, 0.5);
}

TEST(TypedParse, ArmCtrlRejectsTruncatedPayload) {
  std::vector<uint8_t> short_payload = {0x88, 0x13, 0x3C};  // 3 bytes
  EXPECT_FALSE(y::parse_arm_ctrl(short_payload).has_value());
}

// ─── End-to-end: build → parse round-trip ────────────────────────────────────

TEST(RoundTrip, BuildSetMotorParseAsInbound) {
  // The outbound frame uses DEVICE_ID 0xFC, so the inbound parser should NOT
  // consume it. This guards against a bug where outbound and inbound parsers
  // get confused.
  auto outbound = y::build_set_motor(10, 20, 30, 40);
  y::FrameParser p;
  p.feed(outbound.data(), outbound.size());
  EXPECT_FALSE(p.next_frame().has_value());
}
