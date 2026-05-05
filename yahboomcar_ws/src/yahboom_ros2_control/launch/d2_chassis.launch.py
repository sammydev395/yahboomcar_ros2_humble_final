"""D2 chassis-only launch for the X3PLUS.

Brings up:
  - robot_state_publisher (from x3plus_chassis.urdf.xacro)
  - controller_manager (ros2_control_node) with YahboomSystem plugin
  - joint_state_broadcaster (spawned)
  - chassis_controller / mecanum_drive_controller (spawned)

Test after launch:
  ros2 topic pub --once /chassis_controller/reference_unstamped \\
      geometry_msgs/msg/TwistStamped \\
      '{twist: {linear: {x: 0.05}}}'
  ros2 topic echo /joint_states --once
  ros2 topic echo /chassis_controller/odometry --once
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('yahboom_ros2_control')

    # Default URDF path
    urdf_path = PathJoinSubstitution(
        [pkg_share, 'description', 'x3plus_chassis.urdf.xacro'])

    # Default controllers config
    controllers_yaml = PathJoinSubstitution(
        [pkg_share, 'config', 'ros2_controllers.yaml'])

    # Launch args
    urdf_arg = DeclareLaunchArgument(
        'urdf',
        default_value=urdf_path,
        description='Path to URDF/xacro for the X3PLUS chassis.',
    )
    controllers_arg = DeclareLaunchArgument(
        'controllers',
        default_value=controllers_yaml,
        description='Path to ros2_controllers.yaml.',
    )

    # robot_description from xacro at launch time
    robot_description_content = ParameterValue(
        Command(['xacro ', LaunchConfiguration('urdf')]),
        value_type=str,
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description_content}],
    )

    # controller_manager — owns the lifecycle, runs the 100 Hz RT update loop
    controller_manager = Node(
        package='controller_manager',
        executable='ros2_control_node',
        output='screen',
        parameters=[
            {'robot_description': robot_description_content},
            LaunchConfiguration('controllers'),
        ],
    )

    # Spawn joint_state_broadcaster first (no actuator authority — safe to
    # bring up before chassis_controller). It sets up /joint_states from the
    # state interfaces YahboomSystem exports.
    spawn_jsb = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
    )

    # D3: spawn imu_sensor_broadcaster (also state-only, no actuator authority).
    # Reads from YahboomSystem's IMU state interfaces (FUNC_REPORT_IMU_ATT
    # quaternion + FUNC_REPORT_ICM_RAW gyro/accel), publishes /imu_sensor_broadcaster/imu.
    spawn_imu = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['imu_sensor_broadcaster', '--controller-manager', '/controller_manager'],
    )

    # Spawn chassis_controller AFTER joint_state_broadcaster is up. This
    # mirrors Ultra's pattern (controllers chain spawn-order to avoid races
    # where the controller_manager hasn't loaded YahboomSystem yet).
    spawn_chassis = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['chassis_controller', '--controller-manager', '/controller_manager'],
    )

    delay_imu_after_jsb = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_jsb,
            on_exit=[spawn_imu],
        )
    )
    delay_chassis_after_imu = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_imu,
            on_exit=[spawn_chassis],
        )
    )

    return LaunchDescription([
        urdf_arg,
        controllers_arg,
        robot_state_publisher,
        controller_manager,
        spawn_jsb,
        delay_imu_after_jsb,
        delay_chassis_after_imu,
    ])
