#!/usr/bin/env python3
"""
Arm Autopilot Launch File - ROS2 Version
Launches the arm autopilot node with optional dependencies
"""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    # Declare launch arguments
    img_flip_arg = DeclareLaunchArgument(
        'img_flip',
        default_value='false',
        description='Flip the camera image horizontally'
    )
    
    # Get package directories
    arm_autopilot_dir = get_package_share_directory('arm_autopilot')
    
    # Arm Autopilot Node
    arm_autopilot_node = Node(
        package='arm_autopilot',
        executable='autopilot_main.py',
        name='line_detect',
        output='screen',
        parameters=[{
            'img_flip': LaunchConfiguration('img_flip'),
            'VideoSwitch': False,
            # Default PID parameters
            'scale': 1000,
            'Kp': 30.0,
            'Ki': 0.0,
            'Kd': 60.0,
            'linear': 0.10,
            'LaserAngle': 30,
            'ResponseDist': 0.55,
            # Default HSV parameters
            'Calibration': False,
            'Color': 0,
            'Hmin': 0,
            'Hmax': 9,
            'Smin': 85,
            'Smax': 253,
            'Vmin': 126,
            'Vmax': 255,
        }]
    )
    
    return LaunchDescription([
        img_flip_arg,
        arm_autopilot_node,
    ])

