#!/usr/bin/env python3
# encoding: utf-8
"""
Arm Color Transport Common Utilities - ROS2 Version
"""
import threading
from time import sleep

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from cv_bridge import CvBridge

from geometry_msgs.msg import Twist, PoseStamped, Pose
from yahboomcar_msgs.msg import ArmJoint
from sensor_msgs.msg import Image
from std_msgs.msg import Bool, Int32
from visualization_msgs.msg import MarkerArray
from nav2_msgs.action import NavigateToPose
from action_msgs.msg import GoalStatus


class ROSNav:
    """ROS2 Navigation and Arm Control class"""
    
    def __init__(self, node: Node):
        self.node = node
        self.bridge = CvBridge()
        self.InitialParam()
        
        # Declare parameters
        self.node.declare_parameter('img_show', True)
        self.img_show = self.node.get_parameter('img_show').get_parameter_value().bool_value
        
        # Publishers
        self.pub_CmdVel = self.node.create_publisher(Twist, '/cmd_vel', 10)
        self.pub_Arm = self.node.create_publisher(ArmJoint, 'TargetAngle', 1000)
        self.pub_buzzer = self.node.create_publisher(Bool, '/Buzzer', 1)
        self.pub_rgb = self.node.create_publisher(Image, '/Transport/rgb', 1)
        
        # Subscribers
        self.sub_RGBLight = self.node.create_subscription(
            Int32, 'RGBLight', self.RGBLightcallback, 100)
        self.sub_markerArray = self.node.create_subscription(
            MarkerArray, 'color_end_pose', self.getMarker_callback, 10)
        
        # Nav2 Action Client
        self._nav_action_client = ActionClient(
            self.node, NavigateToPose, 'navigate_to_pose')

    def InitialParam(self):
        pst = Pose()
        pst.orientation.w = 1.0
        self.color_pose = {}
        self.start_point = pst
        self.goal_result = 0
        self.joy_action = 0
        self.Transport_status = False
        self.markerArray = MarkerArray()
        self.color_name_list = ['red', 'green', 'blue', 'yellow']

    def RGBLightcallback(self, msg):
        if not isinstance(msg, Int32):
            return
        threading.Thread(target=self.joy_action_update).start()

    def joy_action_update(self):
        self.joy_action += 1
        sleep(0.5)
        self.joy_action = 0

    def PubTargetPoint(self, goal_pose):
        """Send navigation goal using Nav2"""
        self.Transport_status = True
        
        goal_msg = NavigateToPose.Goal()
        goal_msg.pose.header.frame_id = 'map'
        goal_msg.pose.header.stamp = self.node.get_clock().now().to_msg()
        goal_msg.pose.pose = goal_pose
        
        if not self._nav_action_client.wait_for_server(timeout_sec=5.0):
            self.node.get_logger().error('Nav2 action server not available!')
            return
        
        self._send_goal_future = self._nav_action_client.send_goal_async(
            goal_msg, feedback_callback=self.nav_feedback_callback)
        self._send_goal_future.add_done_callback(self.nav_goal_response_callback)

    def nav_feedback_callback(self, feedback_msg):
        """Handle navigation feedback"""
        pass

    def nav_goal_response_callback(self, future):
        """Handle goal response"""
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.node.get_logger().info('Navigation goal rejected')
            return
        
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.nav_result_callback)

    def nav_result_callback(self, future):
        """Handle navigation result"""
        result = future.result()
        if result.status == GoalStatus.STATUS_SUCCEEDED:
            self.goal_result = 3  # SUCCEEDED
            self.Transport_status = False
            self.node.get_logger().info('Navigation succeeded!')
        else:
            self.goal_result = 0
            self.node.get_logger().info(f'Navigation failed with status: {result.status}')

    def getMarker_callback(self, msg):
        if not isinstance(msg, MarkerArray):
            return
        self.color_pose = {}
        for marker in msg.markers:
            if marker.id < 4:
                self.color_pose[self.color_name_list[marker.id]] = marker.pose
            if marker.id == 5:
                self.start_point = marker.pose

    def pubVel(self, x, y, z=0.0):
        twist = Twist()
        twist.linear.x = float(x)
        twist.linear.y = float(y)
        twist.angular.z = float(z)
        self.pub_CmdVel.publish(twist)

    def pubImg(self, img):
        self.pub_rgb.publish(self.bridge.cv2_to_imgmsg(img, "bgr8"))

    def pubBuzzer(self, status):
        msg = Bool()
        msg.data = status
        self.pub_buzzer.publish(msg)

    def pubArm(self, joints, id=10, angle=90, run_time=500):
        armjoint = ArmJoint()
        armjoint.run_time = run_time
        if len(joints) != 0:
            armjoint.joints = [float(j) for j in joints]
        else:
            armjoint.id = id
            armjoint.angle = float(angle)
        self.pub_Arm.publish(armjoint)

    def cancel(self):
        self.pub_CmdVel.publish(Twist())

