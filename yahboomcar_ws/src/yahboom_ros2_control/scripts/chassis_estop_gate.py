#!/usr/bin/env python3
"""Chassis E-stop gate — the ONLY node that feeds chassis_controller.

Fleet E-stop standard (2026-09-27, mirrors Ultra's chassis_stamp_relay
from armpi_ultra ad55c16). arm_teleop owns the SELECT latch and publishes
it on `estop` (std_msgs/Bool, TRANSIENT_LOCAL + RELIABLE). This node
enforces it for the wheels:

  not latched : pass teleop Twist through to the controller unchanged
                (Rosmaster's validated driving path — mecanum 2.53.1 with
                use_stamped_vel:false + reference_timeout:0.5 needs no
                stamping or re-rating, unlike Ultra's).
  latched     : publish zero at 100 Hz and DROP all teleop input, so
                nothing — A held, sticks, turbo — can override the stop.

On latch AND on release a zero goes out immediately, so a pre-stop
motion held in the controller's reference never resumes.

Why a gate and not a second publisher of zeros: two publishers on the
controller's topic race, and teleop_twist_joy (~50 Hz while A is held)
wins — measured on both robots 2026-09-26/27. Enforcement must sit in
the single node between teleop and the controller.

Fails safe: if arm_teleop dies while latched, the last estop=true stays
in effect here and the wheels stay stopped.

Topics are RELATIVE (the launch runs this in /rosmaster):
  in   teleop/cmd_vel                       geometry_msgs/Twist
  in   estop                                std_msgs/Bool (transient-local)
  out  chassis_controller/reference_unstamped  geometry_msgs/Twist
"""
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Bool
from rclpy.qos import QoSProfile, DurabilityPolicy, ReliabilityPolicy

LATCHED_ZERO_HZ = 100.0   # >= controller_manager update_rate


class ChassisEstopGate(Node):
    def __init__(self):
        super().__init__("chassis_estop_gate")
        self.estop = False
        self.pub = self.create_publisher(
            Twist, "chassis_controller/reference_unstamped", 10)
        self.create_subscription(Twist, "teleop/cmd_vel", self.on_cmd, 10)
        self.create_subscription(Bool, "estop", self.on_estop, QoSProfile(
            depth=1, durability=DurabilityPolicy.TRANSIENT_LOCAL,
            reliability=ReliabilityPolicy.RELIABLE))
        self.create_timer(1.0 / LATCHED_ZERO_HZ, self.tick)
        self.get_logger().info(
            "chassis E-stop gate: pass-through; zero @ %.0f Hz while latched"
            % LATCHED_ZERO_HZ)

    def on_estop(self, msg):
        if msg.data != self.estop:
            self.get_logger().warn(
                "E-STOP %s" % ("LATCHED — chassis held at zero, teleop dropped"
                               if msg.data else "released"))
        self.estop = msg.data
        # Drop any held command either way: never resume a pre-stop motion.
        self.pub.publish(Twist())

    def on_cmd(self, msg):
        if self.estop:
            return            # latched: teleop input is dropped entirely
        self.pub.publish(msg)

    def tick(self):
        if self.estop:
            self.pub.publish(Twist())   # unconditional zero while latched


def main():
    rclpy.init()
    try:
        rclpy.spin(ChassisEstopGate())
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
