#!/usr/bin/env python3
"""MoveIt2 Move Group launch file for X3plus arm."""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    # Declare arguments
    declared_arguments = []
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

    return LaunchDescription(declared_arguments + [move_group_node])

