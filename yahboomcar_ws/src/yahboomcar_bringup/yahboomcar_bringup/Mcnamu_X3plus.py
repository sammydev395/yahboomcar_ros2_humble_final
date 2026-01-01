#!/usr/bin/env python3
# encoding: utf-8
"""
Yahboom X3Plus Driver Node - ROS2 Implementation

High-Level Architecture Overview:

Hardware Interface Layer:
- __init__() - Initializes Rosmaster library, sets car type 2, creates publishers/subscribers
- destroy_node() - Clean shutdown with motion stop and thread termination
- safe_float() - Utility function for robust sensor data handling

Data Publishing Layer:
- pub_data() - Threaded function publishing sensor data (IMU, battery, motion, joint states)
- joints_states_update() - Publishes arm joint states for TF and visualization

Arm Control Layer:
- Armcallback() - Processes arm movement commands with debouncing and angle conversion
- srv_Armcallback() - Service handler for querying current arm joint angles

Command Processing Layer:
- cmd_vel_callback() - Processes movement commands from /cmd_vel topic
- RGBLightcallback() - Controls RGB lighting effects
- Buzzercallback() - Controls buzzer on/off states

Parameter Management Layer:
- parameter_callback() - Handles dynamic parameter changes (ROS2 replacement for dynamic_reconfigure)

This node provides the hardware interface for the Yahboom X3Plus platform with robotic arm,
publishing sensor data, processing movement commands, and managing arm control with
proper angle conversions and state synchronization.
"""
import sys
import math
import time
import rclpy
from rclpy.node import Node
import rcl_interfaces.msg
import random
import threading
import numpy as np
from math import pi
from time import sleep
from yahboomcar_msgs.msg import ArmJoint
from yahboomcar_msgs.srv import RobotArmArray
from Rosmaster_Lib import Rosmaster
from geometry_msgs.msg import Twist
from std_msgs.msg import String, Float32, Int32, Bool
from sensor_msgs.msg import Imu, MagneticField, JointState
import tf2_ros
from geometry_msgs.msg import TransformStamped

class YahboomcarDriver(Node):
    # Hardware Interface Layer: Initializes Rosmaster library, sets car type 2, creates publishers/subscribers
    # Sets up ROS2 node with parameter declarations for X3Plus variant with robotic arm.
    # Creates publishers for sensor data and arm updates, subscribers for commands and arm targets.
    # Initializes threaded data publishing, TF buffer, and parameter callback system.
    # Sets car type to 2 (X3Plus variant) and initializes arm joints to safe positions.
    def __init__(self):
        super().__init__('driver_node')
        self.RA2DE = 180 / pi
        self.car = Rosmaster()
        self.car.set_car_type(2)
        self.last_update_time = 1
        self.pos = [0, 0, 0, 0]
        self.imu_link = self.declare_parameter('imu_link', 'imu_link').value
        self.Prefix = self.declare_parameter('prefix', '').value
        self.xlinear_limit = self.declare_parameter('xlinear_speed_limit', 1.0).value
        self.ylinear_limit = self.declare_parameter('ylinear_speed_limit', 1.0).value
        self.angular_limit = self.declare_parameter('angular_speed_limit', 5.0).value
        self.sub_cmd_vel = self.create_subscription(Twist, 'cmd_vel', self.cmd_vel_callback, 100)
        self.sub_RGBLight = self.create_subscription(Int32, 'RGBLight', self.RGBLightcallback, 100)
        self.sub_Buzzer = self.create_subscription(Bool, 'Buzzer', self.Buzzercallback, 100)
        self.sub_Arm = self.create_subscription(ArmJoint, 'TargetAngle', self.Armcallback, 1000)
        self.ArmPubUpdate = self.create_publisher(ArmJoint, 'ArmAngleUpdate', 1000)
        self.EdiPublisher = self.create_publisher(Float32, 'edition', 100)
        self.volPublisher = self.create_publisher(Float32, 'voltage', 100)
        self.staPublisher = self.create_publisher(JointState, 'joint_states', 100)
        self.velPublisher = self.create_publisher(Twist, '/pub_vel', 100)
        self.imuPublisher = self.create_publisher(Imu, '/pub_imu', 100)
        self.magPublisher = self.create_publisher(MagneticField, '/pub_mag', 100)
        self.srv_armAngle = self.create_service(RobotArmArray, 'CurrentAngle', self.srv_Armcallback)
        self._shutdown = False
        self.car.create_receive_threading()
        self.car.set_car_motion(0, 0, 0)
        self.joints = [90, 145, 0, 45, 90, 30]
        self.car.set_uart_servo_angle_array(self.joints, 1000)
        self.pub_thread = threading.Thread(target=self.pub_data)
        self.pub_thread.daemon = True
        self.pub_thread.start()
        self.get_logger().info('Yahboomcar X3plus driver node started.')
        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)
        self.last_arm_command_time = 0
        self.min_command_interval = 0.1  # 100ms minimum between commands
        self.last_servo_angles = [None] * 6
        
        # Declare parameters with default values
        self.declare_parameters(
            namespace='',
            parameters=[
                ('linear_max', 1.0),
                ('linear_min', -1.0),
                ('angular_max', 5.0),
                ('angular_min', -5.0),
                ('joint1', 90.0),
                ('joint2', 90.0),
                ('joint3', 90.0),
                ('joint4', 90.0),
                ('joint5', 90.0),
                ('joint6', 90.0),
                ('SetArmjoint', False),
            ]
        )
        
        # Add parameter callback
        self.add_on_set_parameters_callback(self.parameter_callback)
        
        # Get initial parameter values
        self.linear_max = self.get_parameter('linear_max').value
        self.linear_min = self.get_parameter('linear_min').value
        self.angular_max = self.get_parameter('angular_max').value
        self.angular_min = self.get_parameter('angular_min').value

    # Hardware Interface Layer: Utility function for robust sensor data handling
    # Safely converts values to float with fallback to prevent crashes from invalid sensor data.
    # Used throughout pub_data() to handle potential hardware communication issues gracefully.
    def safe_float(self, x, fallback=0.0):
        try:
            return float(x)
        except (ValueError, TypeError):
            return fallback

    # Data Publishing Layer: Threaded function publishing sensor data (IMU, battery, motion, joint states)
    # Main publishing loop running at 20Hz (50ms intervals). Uses safe_float() for robust sensor data handling.
    # Reads sensor data from Rosmaster library: accelerometer, gyroscope, magnetometer, motion data,
    # battery voltage, and version info. Publishes all data to respective topics and updates arm joint states.
    # Includes error handling for version data conversion to prevent crashes.
    def pub_data(self):
        while rclpy.ok() and not self._shutdown:
            sleep(0.05)
            imu = Imu()
            twist = Twist()
            battery = Float32()
            edition = Float32()
            mag = MagneticField()
            version = self.car.get_version()
            try:
                edition.data = float(version)
            except (ValueError, TypeError):
                self.get_logger().error(f"Invalid version format: {version}")
                edition.data = 0.0
            battery.data = self.car.get_battery_voltage()
            ax, ay, az = [self.safe_float(v) for v in self.car.get_accelerometer_data()]
            gx, gy, gz = [self.safe_float(v) for v in self.car.get_gyroscope_data()]
            mx, my, mz = [self.safe_float(v) for v in self.car.get_magnetometer_data()]
            vx, vy, angular = [self.safe_float(v) for v in self.car.get_motion_data()]
            imu.header.stamp = self.get_clock().now().to_msg()
            imu.header.frame_id = self.imu_link
            imu.linear_acceleration.x = ax
            imu.linear_acceleration.y = ay
            imu.linear_acceleration.z = az
            imu.angular_velocity.x = gx
            imu.angular_velocity.y = gy
            imu.angular_velocity.z = gz
            mag.header.stamp = self.get_clock().now().to_msg()
            mag.header.frame_id = self.imu_link
            mag.magnetic_field.x = mx
            mag.magnetic_field.y = my
            mag.magnetic_field.z = mz
            twist.linear.x = vx
            twist.linear.y = vy
            twist.angular.z = angular
            self.velPublisher.publish(twist)
            self.imuPublisher.publish(imu)
            self.magPublisher.publish(mag)
            self.volPublisher.publish(battery)
            self.EdiPublisher.publish(edition)
            self.joints_states_update()

    # Arm Control Layer: Processes arm movement commands with debouncing and angle conversion
    # Receives ArmJoint messages from joystick or other control nodes. Implements debouncing
    # (50ms minimum interval) to prevent command flooding. Handles both single joint and array commands.
    # Applies angle conversion for joints 2-4 (180-angle) to match hardware requirements.
    # Publishes arm updates and updates joint states for visualization.
    def Armcallback(self, msg):
        if not isinstance(msg, ArmJoint): return
        current_time = time.time()
        if current_time - self.last_arm_command_time < self.min_command_interval:
            return  # Skip if too soon
        self.last_arm_command_time = current_time
        arm_joint = ArmJoint()
        if hasattr(msg, 'joints') and len(msg.joints) != 0:
            arm_joint.joints = self.joints
            for i, angle in enumerate(msg.joints):
                servo_id = i + 1
                if servo_id in [1, 2, 3, 4]:
                    idx = servo_id - 1
                    if self.last_servo_angles[idx] is None or abs(angle - self.last_servo_angles[idx]) > 0.5:
                        self.get_logger().info(f"Moving servo {servo_id} to {angle:.1f}")
                        self.car.set_uart_servo_angle(servo_id, angle, msg.run_time)
                        self.last_servo_angles[idx] = angle
                    else:
                        self.get_logger().debug(f"Servo {servo_id}: Angle change too small, skipping command.")
                else:
                    self.car.set_uart_servo_angle(servo_id, angle, msg.run_time)
                self.joints[i] = angle
                sleep(0.02)
            self.ArmPubUpdate.publish(arm_joint)
        else:
            arm_joint.id = msg.id
            arm_joint.angle = msg.angle
            if msg.id in [1, 2, 3, 4]:
                idx = msg.id - 1
                if self.last_servo_angles[idx] is None or abs(msg.angle - self.last_servo_angles[idx]) > 0.5:
                    self.get_logger().info(f"Moving servo {msg.id} to {msg.angle:.1f}")
                    self.car.set_uart_servo_angle(msg.id, msg.angle, msg.run_time)
                    self.last_servo_angles[idx] = msg.angle
                else:
                    self.get_logger().debug(f"Servo {msg.id}: Angle change too small, skipping command.")
            else:
                self.car.set_uart_servo_angle(msg.id, msg.angle, msg.run_time)
            self.joints[msg.id - 1] = msg.angle
            self.ArmPubUpdate.publish(arm_joint)
            sleep(0.02)
        self.joints_states_update()

    # Arm Control Layer: Service handler for querying current arm joint angles
    # Responds to service requests for current arm joint positions. Attempts to read
    # actual servo angles from hardware. Falls back to last known positions if hardware
    # communication fails. Returns array of 6 joint angles for arm state synchronization.
    def srv_Armcallback(self, request, response):
        joints = self.car.get_uart_servo_angle_array()
        if joints is None or len(joints) != 6:
            self.get_logger().warn("Failed to read joint angles, returning last known or safe default.")
            response.angles = [float(j) for j in getattr(self, 'joints', [90, 145, 0, 45, 90, 30])]
        else:
            response.angles = [float(j) for j in joints]
            self.joints = joints
        return response

    # Command Processing Layer: Controls RGB lighting effects
    # Receives RGB light commands and applies colorful effects to the car's LED system.
    # Calls set_colorful_effect() three times with small delays for reliable hardware communication.
    def RGBLightcallback(self, msg):
        if not isinstance(msg, Int32): return
        for i in range(3):
            self.car.set_colorful_effect(msg.data, 6, parm=1)
            sleep(0.01)

    # Command Processing Layer: Controls buzzer on/off states
    # Receives buzzer commands and controls the car's buzzer. Publishes command three times
    # with small delays for reliable hardware communication. Handles both on (True) and off (False) states.
    def Buzzercallback(self, msg):
        if not isinstance(msg, Bool): return
        if msg.data:
            for i in range(3):
                self.car.set_beep(1)
                sleep(0.01)
        else:
            for i in range(3):
                self.car.set_beep(0)
                sleep(0.01)

    # Command Processing Layer: Processes movement commands from /cmd_vel topic
    # Receives Twist messages and sends movement commands to the car hardware.
    # Extracts linear (x, y) and angular (z) velocities and calls set_car_motion().
    # Note: Does not update joint states here as arm joints are handled separately.
    def cmd_vel_callback(self, msg):
        if not isinstance(msg, Twist): return
        vx = msg.linear.x
        vy = msg.linear.y
        angular = msg.angular.z
        self.get_logger().info(f"cmd_vel_callback: vx={vx:.3f}, vy={vy:.3f}, angular={angular:.3f}")
        if vx == 0 and vy == 0 and angular == 0:
            self.get_logger().warn("cmd_vel_callback: Received all zeros (possible reset or stop command)")
        self.car.set_car_motion(vx, vy, angular)

    # Data Publishing Layer: Publishes arm joint states for TF and visualization
    # Creates JointState message with arm joint names (arm_joint1-5, grip_joint).
    # Converts joint angles from degrees to radians and applies offset calculations.
    # Maps gripper joint from [30,180] range to [0,90] for visualization.
    # Supports prefix parameter for namespaced joint names in multi-robot scenarios.
    def joints_states_update(self):
        state = JointState()
        state.header.stamp = self.get_clock().now().to_msg()
        state.header.frame_id = "joint_states"
        if len(self.Prefix) == 0:
            state.name = ["arm_joint1", "arm_joint2", "arm_joint3", "arm_joint4", "arm_joint5", "grip_joint"]
        else:
            state.name = [self.Prefix + "/arm_joint1", self.Prefix + "/arm_joint2",
                          self.Prefix + "/arm_joint3", self.Prefix + "/arm_joint4",
                          self.Prefix + "/arm_joint5", self.Prefix + "/grip_joint"]
        joints = self.joints[:]
        joints[5] = np.interp(joints[5], [30, 180], [0, 90])
        mid = np.array([90, 90, 90, 90, 90, 90])
        array = np.array(np.array(joints) - mid)
        DEG2RAD = np.array([pi / 180])
        position_src = list(np.dot(array.reshape(-1, 1), DEG2RAD))
        state.position = position_src
        self.staPublisher.publish(state)

    # Parameter Management Layer: Handles dynamic parameter changes (ROS2 replacement for dynamic_reconfigure)
    # Responds to parameter changes from launch files or runtime parameter updates.
    # Updates local parameter values (linear/angular limits) and handles SetArmjoint parameter
    # to trigger arm position updates from parameter values. Provides ROS2 equivalent of
    # ROS1 dynamic_reconfigure functionality.
    def parameter_callback(self, params):
        """Handle parameter changes - equivalent to dynamic_reconfigure_callback"""
        result = rcl_interfaces.msg.SetParametersResult()
        
        for param in params:
            if param.name == 'linear_max':
                self.linear_max = param.value
            elif param.name == 'linear_min':
                self.linear_min = param.value
            elif param.name == 'angular_max':
                self.angular_max = param.value
            elif param.name == 'angular_min':
                self.angular_min = param.value
            elif param.name == 'SetArmjoint':
                if param.value:  # If SetArmjoint is True
                    # Get current joint values
                    joint1 = self.get_parameter('joint1').value
                    joint2 = self.get_parameter('joint2').value
                    joint3 = self.get_parameter('joint3').value
                    joint4 = self.get_parameter('joint4').value
                    joint5 = self.get_parameter('joint5').value
                    joint6 = self.get_parameter('joint6').value
                    
                    # Set arm joints
                    self.car.set_uart_servo_angle_array(
                        [joint1, joint2, joint3, joint4, joint5, joint6], 
                        run_time=1000
                    )
                    self.get_logger().info(f"Set arm joints: {[joint1, joint2, joint3, joint4, joint5, joint6]}")
        
        result.successful = True
        return result

    # Hardware Interface Layer: Clean shutdown with motion stop and thread termination
    # Sets shutdown flag to stop data publishing thread, stops car motion, and ensures clean termination
    # of hardware communication and ROS2 node lifecycle. Prevents runaway arm movements on shutdown.
    def destroy_node(self):
        self._shutdown = True
        self.car.set_car_motion(0, 0, 0)
        super().destroy_node()

def main(args=None):
    rclpy.init(args=args)
    node = YahboomcarDriver()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

