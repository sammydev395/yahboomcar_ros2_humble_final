#!/usr/bin/env python3
"""Launch file for random move demo."""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='arm_moveit_demo',
            executable='random_move.py',
            name='random_move_node',
            output='screen',
        ),
    ])

