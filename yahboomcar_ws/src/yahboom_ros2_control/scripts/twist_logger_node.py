#!/usr/bin/env python3
"""D5 Phase 2 dry-run helper for X3PLUS chassis: subscribe to a Twist
topic, log non-zero messages in human-readable form. Used to verify
chassis teleop output WITHOUT routing it to mecanum_drive_controller.

In phase2_dry_run.launch.py, teleop_twist_joy_node has /cmd_vel remapped
to /dry_run/twist; this node listens on /dry_run/twist. The chassis
controller is never spawned, so even without this remap there's no
actuator path — but the explicit remap + logger gives a clean,
observable verification of what the chassis path WOULD command.

Yahboom port of Ultra's hiwonder_ros2_control/scripts/twist_logger_node.py.
"""
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist


ZERO_THRESH = 0.005  # rad/s and m/s — below this, treat as idle and skip log


class TwistLogger(Node):
    def __init__(self):
        super().__init__("twist_logger")
        self.declare_parameter("topic", "/dry_run/twist")
        topic = str(self.get_parameter("topic").value)
        self.create_subscription(Twist, topic, self._on_twist, 10)
        self.get_logger().info(f"twist_logger listening on {topic}")

    def _on_twist(self, msg: Twist):
        if (
            abs(msg.linear.x) < ZERO_THRESH
            and abs(msg.linear.y) < ZERO_THRESH
            and abs(msg.angular.z) < ZERO_THRESH
        ):
            return  # near-zero — operator released stick
        self.get_logger().info(
            f"[DRY-RUN] Twist  lx={msg.linear.x:+.3f} m/s  "
            f"ly={msg.linear.y:+.3f} m/s  wz={msg.angular.z:+.3f} rad/s"
        )


def main():
    rclpy.init()
    node = TwistLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
