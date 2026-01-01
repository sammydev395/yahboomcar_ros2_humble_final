#!/usr/bin/env python3
"""
Launch file for voice-controlled garbage transport node.
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

    # Voice control node
    voice_ctrl_node = Node(
        package='garbage_identify_yolov11',
        executable='voice_ctrl_garbage.py',
        name='color_transport_node',
        output='screen',
        parameters=[{
            'img_show': LaunchConfiguration('img_show'),
        }],
    )

    return LaunchDescription([
        img_show_arg,
        voice_ctrl_node,
    ])

