#!/usr/bin/env python3
"""
Arm MediaPipe Launch File - ROS2 Version
"""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    arm_ctrl_node = Node(
        package='arm_mediapipe',
        executable='ArmCtrl.py',
        name='hand_ctrl_arm',
        output='screen',
    )
    
    return LaunchDescription([
        arm_ctrl_node,
    ])

