#!/usr/bin/env python3
# encoding: utf-8
"""
ROS2 Navigation and Arm Control utilities for garbage transport.
Migrated from ROS1 to ROS2 Humble.
"""

import rclpy
from rclpy.node import Node
import threading
from time import sleep

from cv_bridge import CvBridge
from geometry_msgs.msg import Twist, Pose, PoseStamped
from nav_msgs.msg import Odometry
from action_msgs.msg import GoalStatusArray
from visualization_msgs.msg import MarkerArray
from sensor_msgs.msg import Image
from std_msgs.msg import Bool, Int32
from yahboomcar_msgs.msg import ArmJoint


class ROSNav:
    """
    ROS2 Navigation and robot control class for garbage transport.
    Handles navigation goals, arm control, and sensor feedback.
    """

    def __init__(self, node: Node):
        self.node = node
        self.logger = node.get_logger()
        self.bridge = CvBridge()
        
        self._init_params()
        self._create_publishers()
        self._create_subscribers()
        
        self.logger.info('ROSNav initialized')

    def _init_params(self):
        """Initialize parameters."""
        self.img_show = self.node.declare_parameter('img_show', True).value
        
        # State variables
        pst = PoseStamped()
        pst.pose.orientation.w = 1.0
        self.color_pose = {}
        self.start_point = pst.pose
        self.goal_result = 0
        self.joy_action = 0
        self.Transport_status = False
        self.RobotRun_status = False
        self.Buzzer_state = False
        self.markerArray = MarkerArray()
        self.color_name_list = ['red', 'green', 'blue', 'yellow']

    def _create_publishers(self):
        """Create ROS2 publishers."""
        self.pub_CmdVel = self.node.create_publisher(Twist, '/cmd_vel', 10)
        self.pub_Arm = self.node.create_publisher(ArmJoint, 'TargetAngle', 10)
        self.pub_goal = self.node.create_publisher(
            PoseStamped, '/goal_pose', 10
        )  # Nav2 uses /goal_pose instead of move_base_simple/goal
        self.pub_buzzer = self.node.create_publisher(Bool, '/Buzzer', 10)
        self.pub_rgb = self.node.create_publisher(Image, '/Transport/rgb', 10)

    def _create_subscribers(self):
        """Create ROS2 subscribers."""
        self.node.create_subscription(
            Int32, 'RGBLight', self.RGBLight_callback, 10
        )
        self.node.create_subscription(
            MarkerArray, 'color_end_pose', self.getMarker_callback, 10
        )
        # Nav2 uses action feedback instead of move_base/result
        self.node.create_subscription(
            GoalStatusArray, '/navigate_to_pose/_action/status',
            self.goal_status_callback, 10
        )

    def RGBLight_callback(self, msg: Int32):
        """Callback for RGB light messages."""
        threading.Thread(target=self._joy_action_update).start()

    def _joy_action_update(self):
        """Update joy action state."""
        self.joy_action += 1
        sleep(0.5)
        self.joy_action = 0

    def PubTargetPoint(self, goal_pose: Pose):
        """Publish a navigation goal."""
        # Move back slightly before navigation
        self.pubVel(-0.2, 0, 0)
        sleep(1)
        self.pubVel(0, -0.2, 0)
        sleep(1)
        self.pubVel(0, 0, 0)
        sleep(1)
        
        self.Transport_status = True
        pose = PoseStamped()
        pose.header.frame_id = 'map'
        pose.header.stamp = self.node.get_clock().now().to_msg()
        pose.pose = goal_pose
        self.pub_goal.publish(pose)
        self.logger.info(f'Published goal: ({goal_pose.position.x:.2f}, '
                        f'{goal_pose.position.y:.2f})')

    def getMarker_callback(self, msg: MarkerArray):
        """Callback for color end pose markers."""
        self.color_pose = {}
        for marker in msg.markers:
            if marker.id < len(self.color_name_list):
                self.color_pose[self.color_name_list[marker.id]] = marker.pose
            if marker.id == 5:
                self.start_point = marker.pose

    def goal_status_callback(self, msg: GoalStatusArray):
        """Callback for navigation goal status (Nav2)."""
        if len(msg.status_list) > 0:
            status = msg.status_list[-1].status
            # Status 4 = SUCCEEDED in Nav2
            if status == 4:
                self.Transport_status = False
                self.goal_result = 3  # Match ROS1 behavior
            elif status == 6:  # ABORTED
                self.goal_result = 4
            elif status == 5:  # CANCELED
                self.goal_result = 2

    def pubVel(self, x: float, y: float, z: float = 0.0):
        """Publish velocity command."""
        twist = Twist()
        twist.linear.x = float(x)
        twist.linear.y = float(y)
        twist.angular.z = float(z)
        self.pub_CmdVel.publish(twist)
        self.RobotRun_status = False

    def pubImg(self, img):
        """Publish image."""
        msg = self.bridge.cv2_to_imgmsg(img, 'bgr8')
        self.pub_rgb.publish(msg)

    def pubBuzzer(self, status: bool):
        """Publish buzzer command."""
        msg = Bool()
        msg.data = status
        self.pub_buzzer.publish(msg)
        self.Buzzer_state = False

    def pubArm(self, joints: list, id: int = 10, angle: float = 90.0, 
               run_time: int = 500):
        """Publish arm joint command."""
        arm_joint = ArmJoint()
        arm_joint.run_time = run_time
        if len(joints) != 0:
            arm_joint.joints = [float(j) for j in joints]
        else:
            arm_joint.id = id
            arm_joint.angle = float(angle)
        self.pub_Arm.publish(arm_joint)

    def cancel(self):
        """Cancel current navigation and stop robot."""
        self.pubVel(0, 0, 0)
        self.logger.info('Navigation cancelled, robot stopped')

