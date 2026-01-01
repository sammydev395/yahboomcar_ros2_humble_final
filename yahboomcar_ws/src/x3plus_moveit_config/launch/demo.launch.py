#!/usr/bin/env python3
"""Demo launch file for X3plus arm with MoveIt2 and RViz visualization."""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # Declare arguments
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            'use_rviz',
            default_value='true',
            description='Start RViz2 with MoveIt plugin'
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation time'
        )
    )

    # Get package directories
    moveit_config_pkg = get_package_share_directory('x3plus_moveit_config')
    description_pkg = get_package_share_directory('yahboomcar_description')

    # Robot description
    robot_description_file = os.path.join(
        description_pkg, 'urdf', 'yahboomcar_X3plus.urdf'
    )
    
    with open(robot_description_file, 'r') as f:
        robot_description_content = f.read()

    robot_description = {'robot_description': robot_description_content}

    # SRDF
    srdf_file = os.path.join(moveit_config_pkg, 'config', 'yahboomcar_X3plus.srdf')
    with open(srdf_file, 'r') as f:
        robot_description_semantic_content = f.read()

    robot_description_semantic = {
        'robot_description_semantic': robot_description_semantic_content
    }

    # Kinematics
    kinematics_yaml_path = os.path.join(
        moveit_config_pkg, 'config', 'kinematics.yaml'
    )
    
    # Joint limits
    joint_limits_yaml_path = os.path.join(
        moveit_config_pkg, 'config', 'joint_limits.yaml'
    )

    # OMPL Planning
    ompl_planning_yaml_path = os.path.join(
        moveit_config_pkg, 'config', 'ompl_planning.yaml'
    )

    # MoveIt controllers
    moveit_controllers_yaml_path = os.path.join(
        moveit_config_pkg, 'config', 'moveit_controllers.yaml'
    )

    # RViz config
    rviz_config = os.path.join(moveit_config_pkg, 'config', 'moveit.rviz')

    # Move group configuration
    move_group_configuration = {
        'publish_robot_description_semantic': True,
        'allow_trajectory_execution': True,
        'capabilities': '',
        'disable_capabilities': '',
        'monitor_dynamics': False,
        'publish_planning_scene': True,
        'publish_geometry_updates': True,
        'publish_state_updates': True,
        'publish_transforms_updates': True,
        'moveit_manage_controllers': True,
    }

    # Robot State Publisher
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[
            robot_description,
            {'use_sim_time': LaunchConfiguration('use_sim_time')},
        ],
    )

    # Joint State Publisher GUI (for testing without real robot)
    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        output='screen',
        parameters=[
            {'use_sim_time': LaunchConfiguration('use_sim_time')},
        ],
    )

    # Move Group Node
    move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=[
            robot_description,
            robot_description_semantic,
            kinematics_yaml_path,
            ompl_planning_yaml_path,
            joint_limits_yaml_path,
            moveit_controllers_yaml_path,
            move_group_configuration,
            {'use_sim_time': LaunchConfiguration('use_sim_time')},
        ],
    )

    # RViz
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config],
        parameters=[
            robot_description,
            robot_description_semantic,
            kinematics_yaml_path,
            {'use_sim_time': LaunchConfiguration('use_sim_time')},
        ],
        condition=IfCondition(LaunchConfiguration('use_rviz')),
    )

    return LaunchDescription(
        declared_arguments + [
            robot_state_publisher_node,
            joint_state_publisher_gui_node,
            move_group_node,
            rviz_node,
        ]
    )

