"""
Rosmaster X3+ Capability Facade Node

Wraps vendor-specific Yahboom ROS2 topics/services into standardized
multi_robot_interfaces for agent-driven coordination.

Vendor topics used:
  /TargetAngle       (yahboomcar_msgs/ArmJoint)  - arm commands
  /ArmAngleUpdate    (yahboomcar_msgs/ArmJoint)  - arm feedback
  /vel_raw           (geometry_msgs/Twist)       - chassis velocity commands
  /cmd_vel           (geometry_msgs/Twist)       - chassis velocity (from joystick)
  /odom_raw          (nav_msgs/Odometry)         - odometry
  /joint_states      (sensor_msgs/JointState)    - joint positions
  /voltage           (std_msgs/Float32)          - battery voltage
"""

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy
import threading
import time

from multi_robot_interfaces.msg import (
    RobotCapability,
    RobotCapabilities,
    Detection,
    DetectionArray,
    TaskStatus,
)
from multi_robot_interfaces.srv import (
    GetRobotStatus,
    MoveArm,
    GripperAction,
    MoveBase,
    StartDetection,
)

from geometry_msgs.msg import Twist
from sensor_msgs.msg import JointState
from std_msgs.msg import Float32
from builtin_interfaces.msg import Time as TimeMsg

ROBOT_NAME = 'rosmaster'


class RosmasterCapabilityNode(Node):
    def __init__(self):
        super().__init__('capability_node', namespace=ROBOT_NAME)

        # --- State ---
        self._joint_states = None
        self._voltage = 0.0
        self._arm_busy = False
        self._base_busy = False
        self._detection_active = False
        self._base_timer = None

        # --- QoS ---
        reliable_qos = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
            depth=10,
        )

        # --- Capability advertisement ---
        self._cap_pub = self.create_publisher(
            RobotCapabilities, 'capabilities', reliable_qos
        )
        self._cap_timer = self.create_timer(2.0, self._publish_capabilities)

        # --- Detection output ---
        self._det_pub = self.create_publisher(
            DetectionArray, 'vision/detections', reliable_qos
        )

        # --- Subscribe to vendor topics ---
        self._joint_sub = self.create_subscription(
            JointState, '/joint_states', self._on_joint_states, 10
        )
        self._voltage_sub = self.create_subscription(
            Float32, '/voltage', self._on_voltage, 10
        )

        # Vendor arm feedback - import dynamically since yahboomcar_msgs
        # is only available inside the Rosmaster container
        try:
            from yahboomcar_msgs.msg import ArmJoint
            self._arm_joint_type = ArmJoint
            self._arm_pub = self.create_publisher(ArmJoint, '/TargetAngle', 10)
            self._arm_fb_sub = self.create_subscription(
                ArmJoint, '/ArmAngleUpdate', self._on_arm_update, 10
            )
            self._has_arm = True
            self.get_logger().info('Yahboom arm interface available')
        except ImportError:
            self._has_arm = False
            self._arm_pub = None
            self.get_logger().warn(
                'yahboomcar_msgs not available - arm control disabled'
            )

        # Vendor base control
        self._base_pub = self.create_publisher(Twist, '/vel_raw', 10)

        # --- Service servers (standardized interface) ---
        self.create_service(GetRobotStatus, 'status', self._handle_status)
        self.create_service(MoveArm, 'arm/move', self._handle_move_arm)
        self.create_service(GripperAction, 'gripper/action', self._handle_gripper)
        self.create_service(MoveBase, 'base/move', self._handle_move_base)
        self.create_service(
            StartDetection, 'vision/start_detection', self._handle_start_detection
        )

        self.get_logger().info(f'Rosmaster capability node started (ns={ROBOT_NAME})')

    # ------------------------------------------------------------------
    # Vendor topic callbacks
    # ------------------------------------------------------------------

    def _on_joint_states(self, msg: JointState):
        self._joint_states = msg

    def _on_voltage(self, msg: Float32):
        self._voltage = msg.data

    def _on_arm_update(self, msg):
        self._arm_busy = False

    # ------------------------------------------------------------------
    # Capability advertisement
    # ------------------------------------------------------------------

    def _publish_capabilities(self):
        msg = RobotCapabilities()
        msg.robot_name = ROBOT_NAME
        msg.stamp = self.get_clock().now().to_msg()

        caps = []

        # Mobile base (always available on X3+)
        base_cap = RobotCapability()
        base_cap.robot_name = ROBOT_NAME
        base_cap.capability_name = 'mobile_base'
        base_cap.status = 'busy' if self._base_busy else 'ready'
        base_cap.supported_actions = ['velocity', 'stop']
        base_cap.hardware_info = 'mecanum_4wd'
        caps.append(base_cap)

        # 6DOF Arm
        arm_cap = RobotCapability()
        arm_cap.robot_name = ROBOT_NAME
        arm_cap.capability_name = 'arm'
        arm_cap.status = 'busy' if self._arm_busy else ('ready' if self._has_arm else 'offline')
        arm_cap.supported_actions = ['move_to_pose', 'named_pose', 'joint']
        arm_cap.hardware_info = '6DOF_yahboom_MoveIt2'
        caps.append(arm_cap)

        # Gripper (last joint of the 6DOF arm)
        grip_cap = RobotCapability()
        grip_cap.robot_name = ROBOT_NAME
        grip_cap.capability_name = 'gripper'
        grip_cap.status = 'ready' if self._has_arm else 'offline'
        grip_cap.supported_actions = ['open', 'close']
        grip_cap.hardware_info = 'servo_gripper'
        caps.append(grip_cap)

        # Camera (Astra Pro RGB-D)
        cam_cap = RobotCapability()
        cam_cap.robot_name = ROBOT_NAME
        cam_cap.capability_name = 'camera_rgbd'
        cam_cap.status = 'ready'
        cam_cap.supported_actions = ['detect_color', 'detect_yolo', 'detect_face', 'detect_gesture']
        cam_cap.hardware_info = 'astra_pro_rgbd'
        caps.append(cam_cap)

        # LiDAR
        lidar_cap = RobotCapability()
        lidar_cap.robot_name = ROBOT_NAME
        lidar_cap.capability_name = 'lidar'
        lidar_cap.status = 'ready'
        lidar_cap.supported_actions = ['scan']
        lidar_cap.hardware_info = 'rplidar'
        caps.append(lidar_cap)

        msg.capabilities = caps
        self._cap_pub.publish(msg)

    # ------------------------------------------------------------------
    # Service handlers
    # ------------------------------------------------------------------

    def _handle_status(self, request, response):
        response.online = True
        response.status = 'ready'
        if self._arm_busy or self._base_busy:
            response.status = 'busy'

        response.capabilities = []
        caps_msg = RobotCapabilities()
        self._publish_capabilities()
        # Build capability list inline
        cap = RobotCapability()
        cap.robot_name = ROBOT_NAME
        cap.capability_name = 'mobile_base'
        cap.status = 'ready'
        cap.supported_actions = ['velocity', 'stop']
        cap.hardware_info = 'mecanum_4wd'
        response.capabilities.append(cap)

        cap2 = RobotCapability()
        cap2.robot_name = ROBOT_NAME
        cap2.capability_name = 'arm'
        cap2.status = 'ready' if self._has_arm else 'offline'
        cap2.supported_actions = ['move_to_pose', 'named_pose']
        cap2.hardware_info = '6DOF_yahboom_MoveIt2'
        response.capabilities.append(cap2)

        cap3 = RobotCapability()
        cap3.robot_name = ROBOT_NAME
        cap3.capability_name = 'gripper'
        cap3.status = 'ready' if self._has_arm else 'offline'
        cap3.supported_actions = ['open', 'close']
        cap3.hardware_info = 'servo_gripper'
        response.capabilities.append(cap3)

        cap4 = RobotCapability()
        cap4.robot_name = ROBOT_NAME
        cap4.capability_name = 'camera_rgbd'
        cap4.status = 'ready'
        cap4.supported_actions = ['detect_color', 'detect_yolo', 'detect_face']
        cap4.hardware_info = 'astra_pro_rgbd'
        response.capabilities.append(cap4)

        response.active_nodes = []
        response.active_topics = []
        return response

    def _handle_move_arm(self, request, response):
        if not self._has_arm:
            response.success = False
            response.message = 'Arm not available (yahboomcar_msgs not loaded)'
            return response

        try:
            if request.mode == 'joint' and len(request.target_position) >= 1:
                # Direct joint control: target_position[0] = joint_id (1-6),
                # target_position[1] = angle in degrees, target_position[2] = run_time ms
                arm_msg = self._arm_joint_type()
                arm_msg.id = int(request.target_position[0])
                arm_msg.angle = request.target_position[1] if len(request.target_position) > 1 else 90.0
                arm_msg.run_time = int(request.target_position[2]) if len(request.target_position) > 2 else 500
                self._arm_busy = True
                self._arm_pub.publish(arm_msg)
                response.success = True
                response.message = (
                    f'Arm joint {arm_msg.id} moving to {arm_msg.angle:.1f} deg '
                    f'(run_time={arm_msg.run_time}ms)'
                )
            elif request.mode == 'named_pose':
                # Named poses as full joints array (degrees): home, up, etc.
                # Joint order: [j1_base, j2_shoulder, j3_elbow, j4_wrist, j5_wrist_rot, j6_gripper]
                # Rest position verified from docs: [90, 145, 0, 45, 90, 30]
                poses = {
                    'home': [90.0, 145.0, 0.0, 45.0, 90.0, 30.0],
                    'up': [90.0, 90.0, 90.0, 90.0, 90.0, 30.0],
                }
                pose_name = request.named_pose or 'home'
                if pose_name not in poses:
                    response.success = False
                    response.message = f'Unknown pose: {pose_name}. Available: {list(poses.keys())}'
                    return response
                arm_msg = self._arm_joint_type()
                arm_msg.run_time = 1500
                arm_msg.joints = poses[pose_name]
                self._arm_busy = True
                self._arm_pub.publish(arm_msg)
                response.success = True
                response.message = f'Arm moving to named pose: {pose_name}'
            else:
                response.success = False
                response.message = (
                    f'Mode "{request.mode}" not yet implemented for Rosmaster. '
                    f'Use mode="joint" with target_position=[joint_id, angle_deg, run_time_ms] '
                    f'or mode="named_pose" with named_pose="home".'
                )
                return response
        except Exception as e:
            response.success = False
            response.message = f'Arm move failed: {str(e)}'

        return response

    def _handle_gripper(self, request, response):
        if not self._has_arm:
            response.success = False
            response.gripper_state = 'offline'
            return response

        try:
            # Gripper is joint6 (id=6) on X3+. 30 deg = closed, 180 deg = open
            GRIPPER_OPEN_DEG = 180.0
            GRIPPER_CLOSED_DEG = 30.0

            arm_msg = self._arm_joint_type()
            arm_msg.id = 6
            arm_msg.run_time = 500

            if request.action == 'open':
                arm_msg.angle = GRIPPER_OPEN_DEG
                response.gripper_state = 'open'
            elif request.action == 'close':
                arm_msg.angle = GRIPPER_CLOSED_DEG
                response.gripper_state = 'closed'
            elif request.action == 'set':
                # position 0.0 = closed, 1.0 = open
                arm_msg.angle = GRIPPER_CLOSED_DEG + (
                    GRIPPER_OPEN_DEG - GRIPPER_CLOSED_DEG
                ) * request.position
                response.gripper_state = 'partial'
            else:
                response.success = False
                response.gripper_state = 'unknown'
                return response

            self._arm_pub.publish(arm_msg)
            response.success = True
        except Exception as e:
            response.success = False
            response.gripper_state = f'error: {str(e)}'

        return response

    def _handle_move_base(self, request, response):
        try:
            if request.mode == 'stop':
                twist = Twist()
                self._base_pub.publish(twist)
                self._base_busy = False
                if self._base_timer:
                    self._base_timer.cancel()
                    self._base_timer = None
                response.success = True
                response.message = 'Base stopped'
                return response

            if request.mode == 'velocity':
                twist = Twist()
                twist.linear.x = request.linear_x
                twist.linear.y = request.linear_y
                twist.angular.z = request.angular_z
                self._base_pub.publish(twist)
                self._base_busy = True

                # Auto-stop after duration
                if request.duration_seconds > 0:
                    if self._base_timer:
                        self._base_timer.cancel()
                    self._base_timer = self.create_timer(
                        request.duration_seconds, self._stop_base_once
                    )

                response.success = True
                response.message = (
                    f'Base moving: linear=({request.linear_x:.2f}, {request.linear_y:.2f}), '
                    f'angular={request.angular_z:.2f}'
                )
            else:
                response.success = False
                response.message = f'Unknown mode: {request.mode}'
        except Exception as e:
            response.success = False
            response.message = f'Base move failed: {str(e)}'

        return response

    def _stop_base_once(self):
        twist = Twist()
        self._base_pub.publish(twist)
        self._base_busy = False
        if self._base_timer:
            self._base_timer.cancel()
            self._base_timer = None
        self.get_logger().info('Base auto-stopped after duration')

    def _handle_start_detection(self, request, response):
        # TODO: Phase 5 - launch vendor detection nodes dynamically
        # For now, return a placeholder indicating the detection type is noted
        response.success = False
        response.message = (
            f'Detection type "{request.detection_type}" noted but vendor node '
            f'launch not yet implemented. Will be added in Phase 5.'
        )
        response.initial_detections = []
        return response


def main(args=None):
    rclpy.init(args=args)
    node = RosmasterCapabilityNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
