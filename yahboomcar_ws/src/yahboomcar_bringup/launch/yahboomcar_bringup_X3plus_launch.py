#!/usr/bin/env python3
"""
Yahboomcar X3Plus Bringup Launch File
Launches the full X3Plus robot with mecanum wheels AND 6-DOF robotic arm.
"""

from ament_index_python.packages import get_package_share_path, get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import Command, LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

import os


def generate_launch_description():
    if not os.environ.get("PRINTED_X3PLUS"):
        os.environ["PRINTED_X3PLUS"] = "1"
        print("=" * 60)
        print("  robot_type = X3Plus (Mecanum + 6-DOF Arm)")
        print("=" * 60)

    # Get package paths
    urdf_tutorial_path = get_package_share_path('yahboomcar_description')
    bringup_pkg = get_package_share_directory('yahboomcar_bringup')
    
    # Use X3Plus URDF
    default_model_path = urdf_tutorial_path / 'urdf/yahboomcar_X3plus.urdf'
    default_rviz_config_path = urdf_tutorial_path / 'rviz/yahboomcar.rviz'

    # Declare launch arguments
    gui_arg = DeclareLaunchArgument(
        name='gui',
        default_value='false',
        choices=['true', 'false'],
        description='Flag to enable joint_state_publisher_gui'
    )
    
    model_arg = DeclareLaunchArgument(
        name='model',
        default_value=str(default_model_path),
        description='Absolute path to robot urdf file'
    )
    
    rviz_arg = DeclareLaunchArgument(
        name='rvizconfig',
        default_value=str(default_rviz_config_path),
        description='Absolute path to rviz config file'
    )
    
    pub_odom_tf_arg = DeclareLaunchArgument(
        'pub_odom_tf',
        default_value='false',
        description='Whether to publish the tf from the original odom to the base_footprint'
    )

    use_rviz_arg = DeclareLaunchArgument(
        'use_rviz',
        default_value='false',
        description='Whether to start RViz'
    )

    # Robot description
    robot_description = ParameterValue(
        Command(['xacro ', LaunchConfiguration('model')]),
        value_type=str
    )

    # Robot State Publisher
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}]
    )

    # Joint State Publisher (for visualization without real robot)
    joint_state_publisher_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        condition=UnlessCondition(LaunchConfiguration('gui'))
    )

    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        condition=IfCondition(LaunchConfiguration('gui'))
    )

    # RViz (optional)
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', LaunchConfiguration('rvizconfig')],
        condition=IfCondition(LaunchConfiguration('use_rviz'))
    )

    # X3Plus Driver Node (with arm support!)
    driver_node = Node(
        package='yahboomcar_bringup',
        executable='Mcnamu_driver_X3plus',
        name='driver_node',
        output='screen',
        parameters=[{
            'xlinear_speed_limit': 0.7,
            'ylinear_speed_limit': 0.7,
            'angular_speed_limit': 3.2,
            'imu_link': 'imu_link',
        }],
        remappings=[
            ('/pub_vel', '/vel_raw'),
            ('/pub_imu', '/imu/imu_raw'),
            ('/pub_mag', '/mag/mag_raw'),
        ],
    )

    # Base Node for odometry
    base_node = Node(
        package='yahboomcar_base_node',
        executable='base_node_X3',
        parameters=[{
            'pub_odom_tf': LaunchConfiguration('pub_odom_tf'),
            'linear_scale_x': 1.0,
            'linear_scale_y': 1.0,
            'angular_scale': 1.0,
        }]
    )

    # IMU Filter
    imu_filter_config = os.path.join(bringup_pkg, 'param', 'imu_filter_param.yaml')
    
    imu_filter_node = Node(
        package='imu_filter_madgwick',
        executable='imu_filter_madgwick_node',
        parameters=[imu_filter_config]
    )

    # EKF for sensor fusion
    ekf_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(
                get_package_share_directory('robot_localization'),
                'launch'
            ),
            '/ekf_x1_x3_launch.py'
        ])
    )

    # X3Plus Joy Controller (with arm control!)
    yahboom_joy_node = Node(
        package='yahboomcar_ctrl',
        executable='yahboom_joy_X3plus',
        name='yahboom_joy',
        output='screen',
        parameters=[{
            'xspeed_limit': 0.7,
            'yspeed_limit': 0.7,
            'angular_speed_limit': 3.2,
        }],
    )

    # Standard Joy Node
    joy_node = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
    )

    return LaunchDescription([
        # Arguments
        gui_arg,
        model_arg,
        rviz_arg,
        pub_odom_tf_arg,
        use_rviz_arg,
        # Nodes
        robot_state_publisher_node,
        joint_state_publisher_node,
        joint_state_publisher_gui_node,
        driver_node,
        base_node,
        imu_filter_node,
        ekf_node,
        yahboom_joy_node,
        joy_node,
        rviz_node,
    ])

