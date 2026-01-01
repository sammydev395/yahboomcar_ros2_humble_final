#!/usr/bin/env python3
"""Launch file for set joint plan demo."""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='arm_moveit_demo',
            executable='set_joint_plan.py',
            name='set_joint_plan_node',
            output='screen',
        ),
    ])

