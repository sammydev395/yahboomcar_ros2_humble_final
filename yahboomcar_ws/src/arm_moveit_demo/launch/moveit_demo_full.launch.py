#!/usr/bin/env python3
"""
Full MoveIt2 demo launch file.
Launches MoveIt2 demo with the simulation to machine bridge.
"""

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    # Get package directories
    moveit_config_pkg = get_package_share_directory('x3plus_moveit_config')
    arm_demo_pkg = get_package_share_directory('arm_moveit_demo')

    # Include MoveIt2 demo launch
    moveit_demo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(moveit_config_pkg, 'launch', 'demo.launch.py')
        ),
    )

    # Simulation to machine bridge
    sim_to_machine = Node(
        package='arm_moveit_demo',
        executable='simulation_to_machine.py',
        name='simulation_to_machine_node',
        output='screen',
    )

    return LaunchDescription([
        moveit_demo,
        sim_to_machine,
    ])

