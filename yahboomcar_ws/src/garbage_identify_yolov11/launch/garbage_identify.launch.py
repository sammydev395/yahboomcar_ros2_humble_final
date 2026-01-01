#!/usr/bin/env python3
"""
Launch file for garbage identification and transport node.
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # Declare arguments
    img_show_arg = DeclareLaunchArgument(
        'img_show',
        default_value='true',
        description='Show image output'
    )

    # Garbage identify node
    garbage_identify_node = Node(
        package='garbage_identify_yolov11',
        executable='garbage_identify_node.py',
        name='garbage_identify_node',
        output='screen',
        parameters=[{
            'img_show': LaunchConfiguration('img_show'),
        }],
    )

    return LaunchDescription([
        img_show_arg,
        garbage_identify_node,
    ])

