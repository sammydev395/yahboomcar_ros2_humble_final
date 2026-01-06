#!/usr/bin/env python3
# encoding: utf-8
"""
ROS2 Unified Voice Control Node for X3Plus
Reads voice commands and publishes to both /cmd_vel (chassis) and /TargetAngle (arm).
Works with regular driver (Mcnamu_X3plus) which subscribes to both topics.
"""

import rclpy
from rclpy.node import Node
from time import sleep
from yahboomcar_msgs.msg import ArmJoint
from geometry_msgs.msg import Twist
from std_msgs.msg import Int32, Bool
from Speech_Lib import Speech


class UnifiedVoiceControl(Node):
    """
    ROS2 Node for unified voice control (chassis + arm).
    Reads voice commands and publishes to appropriate topics.
    """
    
    def __init__(self):
        super().__init__('voice_control')
        
        # Publishers
        self.pub_cmd_vel = self.create_publisher(Twist, '/cmd_vel', 10)
        self.pub_arm = self.create_publisher(ArmJoint, '/TargetAngle', 10)
        self.pub_rgb = self.create_publisher(Int32, '/RGBLight', 10)
        self.pub_buzzer = self.create_publisher(Bool, '/Buzzer', 10)
        
        # Initialize speech module
        self.spe = Speech()
        
        # Initialize arm joint message
        self.arm_joint = ArmJoint()
        self.arm_joint.id = 6
        self.arm_joint.angle = 180.0
        self.arm_joint.run_time = 500
        self.arm_joint.joints = [90.0, 145.0, 0.0, 0.0, 90.0, 31.0]
        
        # Timer to check for voice commands (20 Hz = 0.05s)
        self.create_timer(0.05, self.voice_command_callback)
        
        self.get_logger().info('Unified Voice Control Node initialized')
        self.get_logger().info('Chassis commands: 2/0=stop, 4=forward, 5=back, 6=left, 7=right')
        self.get_logger().info('Arm commands: 39=up, 40=down, 41=left, 42=right, 43=close, 44=open, 45=applaud, 46=nod, 47=pray, 48=kneel, 49=init, 52=dance')
        self.get_logger().info('Light commands: 10=off, 11=red, 12=green, 13=blue, 14=yellow, 15=water, 16=gradient, 17=breathing, 18=electricity')
    
    def voice_command_callback(self):
        """Check for voice commands and execute actions."""
        speech_r = self.spe.speech_read()
        
        if speech_r == 999:  # No command
            return
        
        self.get_logger().info(f'Voice command received: {speech_r}')
        
        # Chassis control commands
        if speech_r == 2 or speech_r == 0:  # Stop
            self.spe.void_write(speech_r)
            self.stop_chassis()
        
        elif speech_r == 4:  # Forward
            self.spe.void_write(speech_r)
            self.move_forward()
        
        elif speech_r == 5:  # Backward
            self.spe.void_write(speech_r)
            self.move_backward()
        
        elif speech_r == 6:  # Turn left
            self.spe.void_write(speech_r)
            self.turn_left()
        
        elif speech_r == 7:  # Turn right
            self.spe.void_write(speech_r)
            self.turn_right()
        
        # Arm control commands
        elif speech_r == 49:  # Init pose
            self.spe.void_write(45)
            self.arm_init_pose()
        
        elif speech_r == 52:  # Dance
            self.spe.void_write(speech_r)
            self.arm_dance()
        
        elif speech_r == 45:  # Applaud
            self.spe.void_write(45)
            self.arm_applaud()
        
        elif speech_r == 46:  # Nod
            self.spe.void_write(45)
            self.arm_nod()
        
        elif speech_r == 39:  # Up
            self.spe.void_write(speech_r)
            self.arm_up()
        
        elif speech_r == 40:  # Down
            self.spe.void_write(speech_r)
            self.arm_down()
        
        elif speech_r == 41:  # Left
            self.spe.void_write(speech_r)
            self.arm_left()
        
        elif speech_r == 42:  # Right
            self.spe.void_write(speech_r)
            self.arm_right()
        
        elif speech_r == 43:  # Clamping (gripper close)
            self.spe.void_write(speech_r)
            self.arm_clamping()
        
        elif speech_r == 44:  # Loosen (gripper open)
            self.spe.void_write(speech_r)
            self.arm_loosen()
        
        elif speech_r == 48:  # Kneel down
            self.spe.void_write(45)
            self.arm_kneel_down()
        
        elif speech_r == 47:  # Pray
            self.spe.void_write(45)
            self.arm_pray()
        
        # RGB Light commands
        elif speech_r == 10:  # Close light
            self.spe.void_write(speech_r)
            self.set_rgb_light(0)
        
        elif speech_r == 11:  # Red light
            self.spe.void_write(speech_r)
            self.set_rgb_light(1)  # Red effect
        
        elif speech_r == 12:  # Green light
            self.spe.void_write(speech_r)
            self.set_rgb_light(2)  # Green effect
        
        elif speech_r == 13:  # Blue light
            self.spe.void_write(speech_r)
            self.set_rgb_light(3)  # Blue effect
        
        elif speech_r == 14:  # Yellow light
            self.spe.void_write(speech_r)
            self.set_rgb_light(4)  # Yellow effect
        
        elif speech_r == 15:  # Water lamps
            self.spe.void_write(speech_r)
            self.set_rgb_light(1)  # Flowing effect
        
        elif speech_r == 16:  # Gradient light
            self.spe.void_write(speech_r)
            self.set_rgb_light(4)  # Gradient effect
        
        elif speech_r == 17:  # Breathing light
            self.spe.void_write(speech_r)
            self.set_rgb_light(3)  # Breathing effect
        
        elif speech_r == 18:  # Display electricity
            self.spe.void_write(speech_r)
            self.set_rgb_light(6)  # Electricity display
    
    # Chassis control methods
    def stop_chassis(self):
        """Stop chassis movement."""
        twist = Twist()
        self.pub_cmd_vel.publish(twist)
    
    def move_forward(self):
        """Move forward for 5 seconds."""
        twist = Twist()
        twist.linear.x = 0.5
        self.pub_cmd_vel.publish(twist)
        sleep(5)
        self.stop_chassis()
    
    def move_backward(self):
        """Move backward for 5 seconds."""
        twist = Twist()
        twist.linear.x = -0.5
        self.pub_cmd_vel.publish(twist)
        sleep(5)
        self.stop_chassis()
    
    def turn_left(self):
        """Turn left for 5 seconds."""
        twist = Twist()
        twist.linear.x = 0.5
        twist.angular.z = 0.2
        self.pub_cmd_vel.publish(twist)
        sleep(5)
        self.stop_chassis()
    
    def turn_right(self):
        """Turn right for 5 seconds."""
        twist = Twist()
        twist.linear.x = 0.5
        twist.angular.z = -0.2
        self.pub_cmd_vel.publish(twist)
        sleep(5)
        self.stop_chassis()
    
    # Arm control methods
    def publish_arm_joints(self, joints):
        """Helper to publish arm joint positions."""
        self.arm_joint.joints = joints
        self.pub_arm.publish(self.arm_joint)
    
    def arm_init_pose(self):
        """Move arm to initial pose."""
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
    
    def arm_up(self):
        """Move arm up."""
        self.publish_arm_joints([94.0, 93.0, 92.0, 88.0, 93.0, 175.0])
    
    def arm_down(self):
        """Move arm down."""
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
        sleep(0.5)
        self.publish_arm_joints([92.0, 6.0, 90.0, 88.0, 93.0, 175.0])
    
    def arm_left(self):
        """Move arm left."""
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
        sleep(0.8)
        self.publish_arm_joints([5.0, 145.0, 0.0, 0.0, 91.0, 32.0])
    
    def arm_right(self):
        """Move arm right."""
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
        sleep(0.8)
        self.publish_arm_joints([179.0, 145.0, 0.0, 0.0, 91.0, 32.0])
    
    def arm_clamping(self):
        """Close gripper."""
        self.publish_arm_joints([89.0, 179.0, 0.0, 0.0, 90.0, 150.0])
    
    def arm_loosen(self):
        """Open gripper."""
        self.publish_arm_joints([89.0, 179.0, 0.0, 0.0, 90.0, 35.0])
    
    def arm_dance(self):
        """Perform arm dance sequence."""
        sequences = [
            [90, 90, 90, 90, 90, 90],
            [90, 60, 120, 60, 90, 90],
            [90, 45, 135, 45, 90, 90],
            [90, 60, 120, 60, 90, 90],
            [90, 90, 90, 90, 90, 90],
            [90, 100, 80, 80, 90, 90],
            [90, 120, 60, 60, 90, 90],
            [90, 135, 45, 45, 90, 90],
            [90, 90, 90, 90, 90, 90],
            [90, 90, 90, 20, 90, 150],
            [90, 90, 90, 90, 90, 90],
            [90, 90, 90, 20, 90, 150],
            [0, 90, 90, 90, 0, 90],
            [0, 90, 180, 0, 0, 90],
            [90, 90, 90, 90, 90, 90],
            [90, 135, 0, 45, 90, 90],
            [90.0, 145.0, 0.0, 0.0, 90.0, 31.0],
        ]
        for joints in sequences:
            self.publish_arm_joints(joints)
            sleep(0.5)
    
    def arm_nod(self):
        """Perform arm nod sequence."""
        for i in range(3):
            self.publish_arm_joints([82.0, 89.0, 93.0, 93.0, 89.0, 32.0])
            sleep(0.5)
            self.publish_arm_joints([82.0, 89.0, 93.0, 33.0, 89.0, 32.0])
            sleep(0.5)
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
    
    def arm_kneel_down(self):
        """Perform arm kneel down sequence."""
        for i in range(3):
            self.publish_arm_joints([90, 11, 179, 0, 90, 33])
            sleep(1)
            self.publish_arm_joints([90, 11, 179, 0, 90, 161])
            sleep(1)
    
    def arm_applaud(self):
        """Perform arm applaud sequence."""
        for i in range(3):
            self.publish_arm_joints([90.0, 145.0, 0.0, 71.0, 90.0, 31.0])
            sleep(0.5)
            self.publish_arm_joints([91.0, 144.0, 0.0, 71.0, 90.0, 168.0])
            sleep(0.5)
        self.publish_arm_joints([90.0, 145.0, 0.0, 0.0, 90.0, 31.0])
    
    def arm_pray(self):
        """Move arm to pray position."""
        self.publish_arm_joints([90, 120, 0, 0, 90, 30])
    
    # RGB Light control
    def set_rgb_light(self, effect):
        """Set RGB light effect."""
        msg = Int32()
        msg.data = effect
        self.pub_rgb.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = UnifiedVoiceControl()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

