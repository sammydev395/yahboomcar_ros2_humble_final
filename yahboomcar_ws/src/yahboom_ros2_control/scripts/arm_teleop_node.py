#!/usr/bin/env python3
"""Gamepad → Float64MultiArray streamer for the X3PLUS arm.

Maps 6 axes onto 6 arm joints, integrating stick deflection × jog_rate × dt
into a tracked `target_positions` array, then republishes a Float64MultiArray
at 50 Hz to `/arm_controller/commands` (forward_command_controller). The
controller writes data[i] to position[i] of the joints listed in
`config/ros2_controllers.yaml arm_controller.joints`.

This is the Yahboom port of Ultra's hiwonder_ros2_control/scripts/
arm_teleop_node.py. Same architecture, same Phase 3 safety nets, retuned
constants for X3PLUS (joint names, URDF limits, axis assignment per
DuLingKer-aligned yahboom_gamepad_map.yaml, gripper protected at vendor
30°-equivalent). HOME button intentionally DISABLED for D5/D6 — the
vendor home pose [90, 145, 0, 45, 90, 30]° includes joint3 at vendor 0°
which maps to URDF +π/2 (URDF upper limit), AT or ABOVE our soft_hi —
HOME redesign is Phase 6 work, not this port.

Deadman: button 1 (B / circle face). Chassis teleop uses button 0
(A / cross face) — separate deadmen so the shared left stick can drive
either chassis or arm with one or the other held, never both at once.
"""
import math

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy, JointState
from std_msgs.msg import Float64MultiArray
from std_srvs.srv import Trigger
from geometry_msgs.msg import TwistStamped


# Joint name → (axis index, jog rate rad/s, URDF lo, URDF hi).
#
# ORDER MUST MATCH config/ros2_controllers.yaml arm_controller's
# `joints:` list — forward_command_controller writes data[i] of the
# incoming Float64MultiArray to that joint, so positional alignment
# between this list and the controller's joints list is mandatory.
#
# Axis assignments mirror Ultra DuLingKer for cross-robot consistency
# (per yahboom_gamepad_map.yaml roles section + feedback memory):
#   LS LR (axis 0) → arm_joint1 (base yaw)
#   LS UD (axis 1) → arm_joint2 (shoulder)
#   RS UD (axis 3) → arm_joint3 (elbow)
#   D-pad LR (axis 6) → arm_joint4 (wrist pitch)
#   RS LR (axis 2) → arm_joint5 (wrist rot)
#   D-pad UD (axis 7) → grip_joint (gripper)
#
# URDF limits from yahboomcar_X3plus.urdf.xacro (verified D2.3 grep).
# Jog rates set to Phase 4 cap (0.10 rad/s) on EVERY joint — operator
# scales up via `-p phase4_jog_rate:=0.X` per the gamepad plan's
# Phase 5 graduated scale-up. NO per-joint differentiation initially:
# vendor X3PLUS code uses uniform 1.4°/30 ms ≈ 0.815 rad/s with no
# slam-prone treatment, and we have no X3PLUS slam history to base
# differentiation on (Ultra's joint2 cap was incident-driven).
JOINT_MAP = [
    # (joint_name,    axis, jog_rate, urdf_lo, urdf_hi)
    ("arm_joint1",     0,    0.10,   -1.571,  1.571),  # LS LR (-π/2..+π/2)
    ("arm_joint2",     1,    0.10,   -1.571,  1.571),  # LS UD (-π/2..+π/2)
    ("arm_joint3",     3,    0.10,   -1.571,  1.571),  # RS UD (-π/2..+π/2)
    ("arm_joint4",     6,    0.10,   -1.571,  1.571),  # D-pad LR (-π/2..+π/2)
    ("arm_joint5",     2,    0.10,   -1.571,  3.142),  # RS LR (-π/2..+π, asymmetric)
    ("grip_joint",     7,    0.10,   -1.571,  0.000),  # D-pad UD (entirely negative)
]

# Phase 3 of docs/YAHBOOM_GAMEPAD_INTEGRATION_PLAN.md — soft per-joint
# limits, TIGHTER than URDF (10° = 0.175 rad margin from each end).
# These are runtime clamping limits; URDF stays in JOINT_MAP for
# traceability.
#
# Notes:
#   - No "joint2 slam-prone tighter cap" treatment — X3PLUS slam path
#     not yet characterized. D6 reviewer obligation: walk arm at URDF
#     extremes (operator-positioned, torque OFF) and identify any
#     self-collision pose; tighten the relevant soft limit if found.
#   - grip_joint range is entirely NEGATIVE (URDF -π/2 to 0). The
#     vendor SDK uses degrees with the gripper "closed" at 30°
#     (yahboom_joy_X3plus.py:212 hard floor) and "open" at 180°. URDF
#     rad → vendor deg conversion via YahboomSystem (offset=90, sign=+1
#     for grip_joint), so grip_joint = -π/2 maps to vendor 0° (jam!),
#     grip_joint = 0 maps to vendor 90°. Soft limits keep us inside
#     a safer sub-range pending D7 calibration.
SOFT_LIMITS = {
    "arm_joint1":  (-1.396,  1.396),  # ±π/2 (URDF) with 10° margin
    "arm_joint2":  (-1.396,  1.396),
    "arm_joint3":  (-1.396,  1.396),
    "arm_joint4":  (-1.396,  1.396),
    "arm_joint5":  (-1.396,  2.967),  # asymmetric: 10° margin from -π/2 and +π
    "grip_joint":  (-1.396, -0.100),  # entirely negative; 10° margin
}

# Per-joint operator-input sign (+1 = direct mapping from joy axis to
# rad-direction; -1 = inverted). Default is +1 for all joints —
# CALIBRATE PER JOINT at D7 single-joint test (push stick in one
# direction, observe physical motion direction; flip sign here if
# inverted from operator intuition). See feedback_check_joy_polarity_first.md.
AXIS_SIGN = {
    "arm_joint1":  +1,  # TODO_CALIBRATE
    "arm_joint2":  +1,  # TODO_CALIBRATE
    "arm_joint3":  +1,  # TODO_CALIBRATE
    "arm_joint4":  +1,  # TODO_CALIBRATE
    "arm_joint5":  +1,  # TODO_CALIBRATE
    "grip_joint":  +1,  # TODO_CALIBRATE
}

# Phase 3 — per-tick max delta safety clamp. At 50 Hz, 0.05 rad/tick =
# 2.5 rad/s effective max. This is a math-glitch tripwire (NaN, axis
# spike past ±1, integration overflow), NOT a normal-operation
# constraint — physical_max_rate is what gates normal operator speed.
MAX_DELTA_PER_TICK = 0.05

# Phase 3 — discontinuity reject threshold. With MAX_DELTA_PER_TICK=0.05
# this should NEVER fire in normal operation — it's a tripwire for math
# glitches (NaN seed, axis spike, integration overflow). On trip:
# freeze target at last good value and log a fault. This alone would
# have stopped the 2026-04-28 Ultra slam.
DISCONTINUITY_THRESHOLD = 0.30

# Physical reach of the bus_servo + ros2_control_node stack on this
# X3PLUS. CAPPING raw_delta in _tick to this rate is what bounds the
# lurch on deadman release: target can't run ahead of what the bus
# can physically deliver.
#
# Phase 4 baseline: 0.80 rad/s. Vendor X3PLUS code runs uniform
# 1.4°/30 ms ≈ 0.815 rad/s in production with no observed issues, so
# 0.80 is a vendor-validated achievable rate. Operator ramps up via
# `-p physical_max_rate:=N` after Phase 5 sign-off.
PHYSICAL_MAX_RATE_DEFAULT = 0.80

DEADMAN_BUTTON = 1   # B / circle face
TURBO_BUTTON = 3     # X / square face — arm turbo (chassis turbo is Y=4)
# Phase 3 — E-stop button, edge-triggered → calls ~/freeze service.
# Default SELECT (button 10). Override via launch arg.
ESTOP_BUTTON_DEFAULT = 10
# Phase 4 — single-joint test mode. "all" (default) = every joint
# responds to its mapped axis. Set to a joint name to gate motion to
# only that joint at a slow phase4_jog_rate cap.
PHASE4_ACTIVE_JOINT_DEFAULT = "all"
PHASE4_JOG_RATE_DEFAULT = 0.0  # 0 = use JOINT_MAP rate; > 0 overrides
# HOME — DISABLED for D5/D6. Phase 6 redesign required because vendor
# home pose [90, 145, 0, 45, 90, 30]° maps via our offset/sign
# conversion to URDF [0, -0.96, +1.57, +0.78, -0.78, -1.05], and
# arm_joint3 = +1.57 hits the URDF upper limit AT/ABOVE our soft_hi
# — straight-line homing would trip the soft-limit walk-back. HOME
# requires either (a) tightened/rebased per-joint conversion that
# puts vendor home inside soft limits, (b) multi-waypoint trajectory
# avoiding the soft-violating zone, or (c) MoveIt-planned homing
# with collision check. All Phase 6.
HOME_BUTTON = 999  # never matches; HOME stays inert

PUBLISH_HZ = 50.0
INPUT_DEADZONE = 0.10
TURBO_FACTOR = 2.0

# LIVE-mode publish-log threshold (rad). When ANY joint's target has
# moved >= this since the last LIVE log line, log a "[LIVE] publishing
# Float64MultiArray ..." entry. Sparse so we don't spam at 50 Hz.
LIVE_LOG_THRESHOLD = 0.005

# Vendor home pose in JOINT_MAP order (URDF radians). Computed from
# vendor degrees [90, 145, 0, 45, 90, 30] using:
#   urdf_rad = (vendor_deg - offset_deg) / (axis_sign * 180/π)
# with offset/sign per yahboom_system.hpp kArmVendorOffsetDeg/kArmAxisSign:
#   arm_joint1: (90-90)/(-1*180/π)  = 0.000
#   arm_joint2: (145-90)/(-1*180/π) = -0.960
#   arm_joint3: (0-90)/(-1*180/π)   = +1.571   ← AT URDF limit, > soft_hi
#   arm_joint4: (45-90)/(-1*180/π)  = +0.785
#   arm_joint5: (90-135)/(+1*180/π) = -0.785
#   grip_joint: (30-90)/(+1*180/π)  = -1.047
# Currently UNUSED (HOME_BUTTON=999). Phase 6 work needed before this
# becomes a viable target; see HOME_BUTTON comment block.
VENDOR_HOME_RAD = [0.000, -0.960, +1.571, +0.785, -0.785, -1.047]


class ArmTeleop(Node):
    def __init__(self):
        super().__init__("arm_teleop")
        # Phase 2 of docs/YAHBOOM_GAMEPAD_INTEGRATION_PLAN.md. When
        # dry_run=True the node logs every Float64MultiArray it WOULD
        # have published instead of publishing — used to verify
        # input → command mapping without an actuator path.
        self.declare_parameter("dry_run", False)
        self.dry_run = bool(self.get_parameter("dry_run").value)
        # Phase 3 — E-stop button override.
        self.declare_parameter("estop_button", ESTOP_BUTTON_DEFAULT)
        self.estop_button = int(self.get_parameter("estop_button").value)
        # Phase 4 — single-joint test mode.
        self.declare_parameter("active_joint", PHASE4_ACTIVE_JOINT_DEFAULT)
        self.active_joint = str(self.get_parameter("active_joint").value)
        self.declare_parameter("phase4_jog_rate", PHASE4_JOG_RATE_DEFAULT)
        self.phase4_jog_rate = float(self.get_parameter("phase4_jog_rate").value)
        # Physical max rate the bus stack can deliver. Caps target
        # advance per tick so target NEVER runs ahead of what the
        # servo can physically achieve — without this, fast operator
        # input with high jog rates accumulates "stored motion" that
        # lurches out on deadman release. Hard safety cap.
        self.declare_parameter("physical_max_rate", PHYSICAL_MAX_RATE_DEFAULT)
        self.physical_max_rate = float(self.get_parameter("physical_max_rate").value)
        # Sanity: if active_joint isn't in JOINT_MAP, fail loudly.
        if self.active_joint != "all":
            valid = {j[0] for j in JOINT_MAP}
            if self.active_joint not in valid:
                raise ValueError(
                    f"active_joint='{self.active_joint}' not in JOINT_MAP. "
                    f"Valid choices: 'all' or one of {sorted(valid)}")

        self.joint_names = [j[0] for j in JOINT_MAP]
        self.target = [0.0] * len(self.joint_names)
        self.have_state = False
        # Phase 3 — keep latest /joint_states for ~/freeze service so we
        # snap target = current actual pose, not last commanded target.
        self.last_state = None

        self.create_subscription(JointState, "/joint_states", self._on_state, 10)
        self.create_subscription(Joy, "/joy", self._on_joy, 10)
        # Publisher for arm_controller (forward_command_controller).
        # Float64MultiArray.data[i] maps directly to position[i] of the
        # joints listed in config/ros2_controllers.yaml arm_controller.
        self.pub = self.create_publisher(
            Float64MultiArray, "/arm_controller/commands", 10)

        # Phase 5+ — chassis E-stop publisher. Mecanum_drive_controller
        # subscribes to TwistStamped on /chassis_controller/reference
        # (Humble convention; Iron+ defaults to TwistStamped). When the
        # operator hits E-stop, we override teleop_twist_joy's current
        # output by publishing a zero TwistStamped so wheels stop too.
        self.chassis_stop_pub = self.create_publisher(
            TwistStamped, "/chassis_controller/reference", 10)

        # Phase 3 — ~/freeze service: snap target to current state +
        # publish once. The recovery primitive that REPLACES the unsafe
        # HOME teleport.
        self._freeze_srv = self.create_service(
            Trigger, "~/freeze", self._on_freeze)

        self.last_joy = None
        # Edge-trigger state for E-stop button so a stuck/bouncing button
        # can't spam ~/freeze at 50 Hz.
        self._estop_was_pressed = False
        # Dry-run log de-dupe: only log when target changed since last log.
        self._last_dry_run_positions = None
        # LIVE-mode publish logging — sparse (threshold-based).
        self._last_live_log_positions = None
        self.dt = 1.0 / PUBLISH_HZ
        self.create_timer(self.dt, self._tick)

        mode = "DRY-RUN (logs only, never publishes)" if self.dry_run else "LIVE"
        BOLD = "\x1b[1m"
        OFF = "\x1b[0m"
        log = self.get_logger().info
        log("=" * 76)
        log(f"arm_teleop READY [{mode}] — Phase 3 safety nets ACTIVE")
        log("=" * 76)
        log("SOFT LIMITS (rad) — TIGHTER than URDF; runtime clamp source:")
        for name, _axis, _jog, urdf_lo, urdf_hi in JOINT_MAP:
            soft_lo, soft_hi = SOFT_LIMITS[name]
            log(
                f"  {name:<14} soft [{BOLD}{soft_lo:+.3f}, {soft_hi:+.3f}{OFF}]"
                f"   URDF [{urdf_lo:+.3f}, {urdf_hi:+.3f}]"
            )
        log(
            f"PHYSICAL MAX RATE:    {BOLD}{self.physical_max_rate:.3f} rad/s{OFF}"
            f"   caps target advance — release deadman = arm stops within 1 tick"
        )
        log(
            f"PER-TICK CLAMP:       {BOLD}{MAX_DELTA_PER_TICK:.3f} rad/tick{OFF}"
            f"   (~{MAX_DELTA_PER_TICK * PUBLISH_HZ:.1f} rad/s @ {PUBLISH_HZ:.0f} Hz, math-glitch upper bound)"
        )
        log(f"DISCONTINUITY REJECT: {BOLD}{DISCONTINUITY_THRESHOLD:.3f} rad{OFF}   (tripwire — should NEVER fire in normal ops)")
        log(f"E-STOP BUTTON:        {BOLD}{self.estop_button} (Select){OFF}   edge-triggered → ~/freeze (stops ARM + CHASSIS)")
        log(f"DEADMAN BUTTON:       {DEADMAN_BUTTON} (B)   required for arm motion")
        log(f"TURBO BUTTON:         {TURBO_BUTTON} (X)   {TURBO_FACTOR}× rate when held with deadman")
        log(f"HOME BUTTON:          DISABLED (Phase 6 redesign required — vendor home maps outside soft limits)")
        if self.active_joint != "all":
            log("=" * 76)
            log(f"{BOLD}PHASE 4 SINGLE-JOINT MODE: only '{self.active_joint}' responds to stick input.{OFF}")
            if self.phase4_jog_rate > 0.0:
                log(f"{BOLD}PHASE 4 JOG RATE OVERRIDE: {self.phase4_jog_rate:.3f} rad/s{OFF}")
            log("Every other joint frozen at its current pose.")
        log("=" * 76)
        log(f"Hold B (button {DEADMAN_BUTTON}) + sticks/D-pad to jog. "
            f"Press button {self.estop_button} (Select) to E-stop "
            f"(stops ARM + CHASSIS). HOME disabled.")

    def _on_state(self, msg: JointState):
        # Always keep latest /joint_states for the ~/freeze service.
        self.last_state = msg
        if self.have_state:
            return
        # First state msg: seed target from real arm pose so we don't snap.
        for i, jn in enumerate(self.joint_names):
            try:
                idx = msg.name.index(jn)
                self.target[i] = msg.position[idx]
            except ValueError:
                pass
        self.have_state = True
        self.get_logger().info(f"seeded target from /joint_states: {self.target}")

    def _on_joy(self, msg: Joy):
        self.last_joy = msg

    def _tick(self):
        if not self.have_state or self.last_joy is None:
            return
        joy = self.last_joy
        # Phase 3 — E-stop (edge-triggered). Checked BEFORE deadman so
        # it overrides any in-progress motion.
        estop_pressed = (
            len(joy.buttons) > self.estop_button
            and bool(joy.buttons[self.estop_button])
        )
        if estop_pressed and not self._estop_was_pressed:
            self._estop_was_pressed = True
            self.get_logger().warn(
                f"[E-STOP] gamepad button {self.estop_button} pressed "
                f"— calling internal freeze")
            self._freeze()
            return  # this tick: only the freeze publish, nothing else
        if not estop_pressed:
            self._estop_was_pressed = False  # arm for next press

        deadman_held = (
            len(joy.buttons) > DEADMAN_BUTTON
            and bool(joy.buttons[DEADMAN_BUTTON])
        )
        if not deadman_held:
            # Deadman released — DON'T early-return. Publish the unchanged
            # target so /arm_controller/commands has a fresh message every
            # tick. forward_command_controller's update() is a no-op when
            # no command has been received over the topic, but as soon as
            # ANY command arrives it begins writing whatever it last
            # received to its command interfaces every cycle. Without this
            # 50 Hz idle-republish the controller's command_interfaces
            # could end up at default-zero values (D7.5 first-launch
            # observation 2026-05-07: joint2 oscillated when no publisher
            # was attached). Republishing the seeded target keeps cmd ==
            # state, so YahboomSystem.write() emits no-op heartbeats and
            # the arm holds.
            self._publish_target(time_from_start_sec=self.dt)
            return  # no motion change beyond the idle-publish

        turbo = (
            len(joy.buttons) > TURBO_BUTTON and joy.buttons[TURBO_BUTTON]
        )
        for i, (name, axis, jog, _urdf_lo, _urdf_hi) in enumerate(JOINT_MAP):
            # Phase 4 single-joint gating.
            if self.active_joint != "all" and name != self.active_joint:
                continue
            if axis >= len(joy.axes):
                continue
            v = joy.axes[axis]
            if abs(v) < INPUT_DEADZONE:
                continue
            effective_jog = (
                self.phase4_jog_rate if self.phase4_jog_rate > 0.0 else jog
            )
            rate = effective_jog * (TURBO_FACTOR if turbo else 1.0)
            sign = AXIS_SIGN.get(name, +1)
            raw_delta = v * sign * rate * self.dt
            # Physical max rate cap — keeps target from running ahead
            # of what the bus stack can physically deliver.
            phys_cap = self.physical_max_rate * self.dt
            if raw_delta > phys_cap:
                raw_delta = phys_cap
            elif raw_delta < -phys_cap:
                raw_delta = -phys_cap
            # Phase 3 — per-tick math-glitch safety clamp.
            if raw_delta > MAX_DELTA_PER_TICK:
                raw_delta = MAX_DELTA_PER_TICK
            elif raw_delta < -MAX_DELTA_PER_TICK:
                raw_delta = -MAX_DELTA_PER_TICK
            proposed = self.target[i] + raw_delta
            # Phase 3 — soft per-joint limits + walk-back logic.
            # If state STARTS outside soft envelope (e.g. arm at vendor
            # home pose with arm_joint3 at +1.571 vs soft_hi +1.396),
            # DON'T snap-clamp to boundary — that converts a tiny per-
            # tick delta into a multi-rad jump → discontinuity reject.
            # Instead: only allow proposed motion that walks state
            # TOWARD the safe envelope; refuse motion further out.
            soft_lo, soft_hi = SOFT_LIMITS[name]
            if self.target[i] < soft_lo:
                # Below soft_lo — only let proposed move upward.
                proposed = max(self.target[i], proposed)
            elif self.target[i] > soft_hi:
                # Above soft_hi — only let proposed move downward.
                proposed = min(self.target[i], proposed)
            else:
                # State in safe envelope — normal soft-limit clamping.
                if proposed < soft_lo:
                    proposed = soft_lo
                if proposed > soft_hi:
                    proposed = soft_hi
            # Phase 3 — discontinuity reject. Should NEVER fire.
            actual_delta = proposed - self.target[i]
            if abs(actual_delta) > DISCONTINUITY_THRESHOLD:
                self.get_logger().error(
                    f"[FAULT] discontinuity reject on {name}: "
                    f"Δ={actual_delta:+.4f} rad > {DISCONTINUITY_THRESHOLD}; "
                    f"freezing at {self.target[i]:.4f} rad. "
                    f"axis={axis} v={v:.3f} rate={rate:.3f}")
                continue
            self.target[i] = proposed

        self._publish_target(time_from_start_sec=self.dt)

    def _publish_target(self, time_from_start_sec: float):
        # time_from_start_sec is no longer wire-meaningful (forward_command_
        # controller has no concept of trajectory timing). Kept in the
        # signature for log line annotation.
        msg = Float64MultiArray()
        positions = list(self.target)
        msg.data = positions
        if self.dry_run:
            # Squelch identical re-publishes — without this, holding
            # deadman with no stick input spams 50 Hz of identical
            # zero-target lines. None means "log next publish unconditionally".
            if self._last_dry_run_positions == positions:
                return
            self._last_dry_run_positions = positions
            log_str = ", ".join(
                f"{n}={p:+.3f}" for n, p in zip(self.joint_names, positions)
            )
            self.get_logger().info(
                f"[DRY-RUN] WOULD publish Float64MultiArray "
                f"tfs={time_from_start_sec:.2f}s | {log_str}"
            )
            return
        # LIVE — sparse log when target moved >= LIVE_LOG_THRESHOLD.
        if LIVE_LOG_THRESHOLD > 0.0:
            if (self._last_live_log_positions is None
                    or any(abs(c - p) >= LIVE_LOG_THRESHOLD
                           for c, p in zip(positions, self._last_live_log_positions))):
                self._last_live_log_positions = positions
                log_str = ", ".join(
                    f"{n}={p:+.3f}" for n, p in zip(self.joint_names, positions)
                )
                self.get_logger().info(
                    f"[LIVE] publishing Float64MultiArray "
                    f"tfs={time_from_start_sec:.2f}s | {log_str}"
                )
        self.pub.publish(msg)

    # ---- Phase 3 freeze service ------------------------------------------
    def _on_freeze(self, request, response):
        """std_srvs/Trigger handler for ~/freeze."""
        self._freeze()
        response.success = True
        response.message = (
            f"frozen at {self.target}" if self.have_state
            else "frozen — no /joint_states yet, holding last target"
        )
        return response

    def _freeze(self):
        """Internal: hold current pose; publish once with long tfs.

        DRY-RUN mode: hold self.target (last commanded). The /joint_states
        feedback in dry-run + Phase 2 launch is the perfect-tracker echo
        that YahboomSystem provides; it'd say "wherever the operator
        jogged to," same as self.target. Holding target is "stop where
        you are" — what freeze means.

        LIVE mode: snap target to /joint_states (real servo readback).
        Safer than holding the last commanded target — last_state is
        where the arm ACTUALLY is, target is where we WANTED it to go.
        Freezing at last_state means "hold where the arm actually is".
        """
        if self.dry_run:
            self.get_logger().warn(
                f"[FREEZE / DRY-RUN] holding last commanded target: {self.target}")
        elif self.last_state is None:
            self.get_logger().warn(
                "[FREEZE] no /joint_states yet — holding last target")
        else:
            for i, jn in enumerate(self.joint_names):
                try:
                    idx = self.last_state.name.index(jn)
                    self.target[i] = self.last_state.position[idx]
                except ValueError:
                    pass
            self.get_logger().warn(
                f"[FREEZE] target snapped to /joint_states: {self.target}")
        # Force the next publish to log even if dedupe would suppress.
        self._last_dry_run_positions = None
        self._publish_target(time_from_start_sec=0.5)
        # Also stop the chassis. Publish zero TwistStamped to override
        # whatever teleop_twist_joy is currently asserting. Sent
        # regardless of dry_run; in dry-run it's harmless (no controller
        # spawned to receive it), in LIVE it's the actual chassis E-stop.
        zero_ts = TwistStamped()
        zero_ts.header.stamp = self.get_clock().now().to_msg()
        zero_ts.header.frame_id = "base_link"
        # All linear/angular fields default to 0.0
        self.chassis_stop_pub.publish(zero_ts)
        self.get_logger().warn(
            "[FREEZE] chassis zero-TwistStamped published to "
            "/chassis_controller/reference")


def main():
    rclpy.init()
    node = ArmTeleop()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
