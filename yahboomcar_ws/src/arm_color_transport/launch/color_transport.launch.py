#!/usr/bin/env python3
"""
Arm Color Transport Launch File - ROS2 Version
"""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    img_show_arg = DeclareLaunchArgument(
        'img_show',
        default_value='true',
        description='Show image window'
    )
    
    color_transport_node = Node(
        package='arm_color_transport',
        executable='transport_main.py',
        name='color_transport',
        output='screen',
        parameters=[{
            'img_show': LaunchConfiguration('img_show'),
        }]
    )
    
    return LaunchDescription([
        img_show_arg,
        color_transport_node,
    ])

