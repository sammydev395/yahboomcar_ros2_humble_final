#!/usr/bin/env python3
"""Launch file for simulation to machine bridge."""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='arm_moveit_demo',
            executable='simulation_to_machine.py',
            name='simulation_to_machine_node',
            output='screen',
        ),
    ])

