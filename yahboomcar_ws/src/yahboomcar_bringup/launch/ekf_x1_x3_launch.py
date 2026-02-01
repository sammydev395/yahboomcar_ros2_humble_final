#!/usr/bin/env python3
"""
EKF launch for X1/X3 robots. Uses odom_raw and IMU from bringup.
Custom launch because robot_localization from apt does not ship ekf_x1_x3_launch.py.
"""
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
import launch_ros.actions


def generate_launch_description():
    bringup_share = get_package_share_directory('yahboomcar_bringup')
    params_file = os.path.join(bringup_share, 'param', 'ekf_x1_x3.yaml')
    if not os.path.isfile(params_file):
        # Fallback to robot_localization default params if our file is not installed
        rl_share = get_package_share_directory('robot_localization')
        params_file = os.path.join(rl_share, 'params', 'ekf.yaml')

    return LaunchDescription([
        launch_ros.actions.Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[params_file],
        ),
    ])
