"""D8.3 master launch — single entry point for the full X3PLUS stack.

Includes the proven D8.2 PASS combined-controller stack
(phase5_combined.launch.py = controller_manager + JSB + arm_controller +
chassis_controller + joy_node + arm_teleop + teleop_twist_joy) AS-IS,
then adds the YDLidar TG (4ROS variant) on top:

  - ydlidar_ros2_driver_node  (LifecycleNode, auto-configure → auto-activate)
  - static_transform_publisher  base_link → laser  (4ROS TF, no pitch)

NOT in this launch (intentionally):
  - vendor Mcnamu_driver_X3plus / yahboom_joy_X3plus / base_node_X3
  - Astra Pro camera (yahboomcar_astra)         — side launch from start_node_inside_container.sh
  - rosmaster_capability multi-robot facade     — side launch
  - web_video_server (port 8090)                — side launch in a clean shell
                                                  (avoids library_ws OpenCV 4.10
                                                  overlay; see MEMORY.md → Web
                                                  Video Streaming)
  - imu_filter_madgwick / ekf_node              — D9 (re-add downstream of
                                                  imu_sensor_broadcaster)
  - voice / CI1302 driver                       — D11

Pre-flight:
  1. /dev/ydlidar symlink present (vendor's ydlidar.rules — single CP210x
     on the bus, no dual-CH340-style ambiguity).
  2. /dev/yahboom_stm32 symlink resolves to the STM32 CH340 (NOT the
     CI1302 voice module). See memory project_yahboom_dual_ch340_devpath.md
     and the devpath-pinned udev rule in provision/jetson/.
  3. library_ws is sourced before invoking this launch (so the
     ydlidar_ros2_driver package is on AMENT_PREFIX_PATH).

Lifecycle activation note:
  The vendor ydlidar_ros2_driver_node is a LifecycleNode and its shipped
  ydlidar_launch.py spawns it WITHOUT auto-configure / auto-activate —
  the node sits in UNCONFIGURED forever and never publishes /scan. We
  fix this here by emitting two ChangeState events:
    - On launch start: UNCONFIGURED → INACTIVE (configure)
    - When that completes: INACTIVE → ACTIVE (activate)
  The second is wired via OnStateTransition so we don't race the first.

Static-TF source:
  base_link → laser at xyz=(0.0435, 5.258E-05, 0.11) rpy=(0,0,0).
  Extracted from vendor laser_bringup_no_odom_launch.py for the
  RPLIDAR_TYPE='4ROS' branch (= our YDLidar TG). Note the RPLidar branch
  uses rpy=(3.14,0,0) — different upside-down mount geometry — so do
  NOT cargo-cult that into this launch.

Args (all forwarded to phase5_combined.launch.py + a couple lidar-side):
  arm_jog_rate       (default 0.40 rad/s — D8.1 50%, ×2 with X turbo)
  linear_scale       (default 0.10 m/s)
  linear_turbo_scale (default 0.30 m/s)
  angular_scale      (default 0.30 rad/s)
  angular_turbo_scale(default 1.00 rad/s)
  active_joint       (default "all")
  device_id          (default 0 = /dev/yahboom_joy)
  enable_lidar       (default "true" — set "false" for bench tests with
                      no YDLidar plugged in)
  ydlidar_params     (default = vendor params yaml from library_ws)
"""

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    EmitEvent,
    IncludeLaunchDescription,
    LogInfo,
    RegisterEventHandler,
)
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import LifecycleNode, Node
from launch_ros.event_handlers import OnStateTransition
from launch_ros.events.lifecycle import ChangeState
from launch_ros.substitutions import FindPackageShare
import lifecycle_msgs.msg


def generate_launch_description():
    pkg_share = FindPackageShare('yahboom_ros2_control')
    ydlidar_share = FindPackageShare('ydlidar_ros2_driver')

    # Defaults match phase5_combined.launch.py — re-declared here so
    # `ros2 launch ... --show-args` lists them at the master entry point
    # and so robot-test.sh can pass them directly.
    arm_jog_rate_arg = DeclareLaunchArgument(
        'arm_jog_rate', default_value='0.40',
        description='Arm jog rate cap (rad/s) for arm_teleop. D8.1 PASS at 0.40 ×2 turbo.')
    linear_scale_arg = DeclareLaunchArgument(
        'linear_scale', default_value='0.10',
        description='Chassis linear cap (m/s).')
    linear_turbo_scale_arg = DeclareLaunchArgument(
        'linear_turbo_scale', default_value='0.30',
        description='Chassis linear cap (m/s) when Y (turbo) is held.')
    angular_scale_arg = DeclareLaunchArgument(
        'angular_scale', default_value='0.30',
        description='Chassis angular cap (rad/s).')
    angular_turbo_scale_arg = DeclareLaunchArgument(
        'angular_turbo_scale', default_value='1.00',
        description='Chassis angular cap (rad/s) when Y (turbo) is held.')
    active_joint_arg = DeclareLaunchArgument(
        'active_joint', default_value='all',
        description='arm_teleop active_joint. "all" = full JOINT_MAP.')
    device_id_arg = DeclareLaunchArgument(
        'device_id', default_value='0',
        description='Joystick index (0 = /dev/input/js0 = /dev/yahboom_joy).')
    enable_lidar_arg = DeclareLaunchArgument(
        'enable_lidar', default_value='true',
        description='Set "false" to skip YDLidar (bench tests without /dev/ydlidar).')
    ydlidar_params_arg = DeclareLaunchArgument(
        'ydlidar_params',
        default_value=PathJoinSubstitution(
            [ydlidar_share, 'params', 'ydlidar.yaml']),
        description='Path to ydlidar_ros2_driver params yaml (defaults to vendor yaml).')

    pre_flight_banner = LogInfo(
        msg=[
            '\n',
            '═════════════════════════════════════════════════════════════\n',
            '  D8.3 MASTER LAUNCH — full X3PLUS stack\n',
            '─────────────────────────────────────────────────────────────\n',
            '  Includes phase5_combined (controllers + arm + chassis + joy)\n',
            '  Adds:\n',
            '    - ydlidar_ros2_driver_node (LifecycleNode, auto-activate)\n',
            '    - static_tf base_link → laser (4ROS, no pitch)\n',
            '  Side launches NOT in this file (handled by\n',
            '  start_node_inside_container.sh):\n',
            '    - rosmaster_capability\n',
            '    - yahboomcar_astra\n',
            '    - web_video_server (clean shell, no library_ws overlay)\n',
            '─────────────────────────────────────────────────────────────\n',
            '  active_joint        = ', LaunchConfiguration('active_joint'), '\n',
            '  arm_jog_rate        = ', LaunchConfiguration('arm_jog_rate'), ' rad/s\n',
            '  chassis linear      = ', LaunchConfiguration('linear_scale'),
            ' m/s (turbo: ', LaunchConfiguration('linear_turbo_scale'), ')\n',
            '  chassis angular     = ', LaunchConfiguration('angular_scale'),
            ' rad/s (turbo: ', LaunchConfiguration('angular_turbo_scale'), ')\n',
            '  enable_lidar        = ', LaunchConfiguration('enable_lidar'), '\n',
            '  ydlidar_params      = ', LaunchConfiguration('ydlidar_params'), '\n',
            '═════════════════════════════════════════════════════════════\n',
        ],
    )

    # Re-use the D8.2 PASS combined-controller stack verbatim. Forward
    # every applicable launch arg so master.launch.py is a strict
    # superset of phase5_combined.launch.py.
    phase5_combined = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_share, 'launch', 'phase5_combined.launch.py'])),
        launch_arguments={
            'arm_jog_rate': LaunchConfiguration('arm_jog_rate'),
            'linear_scale': LaunchConfiguration('linear_scale'),
            'linear_turbo_scale': LaunchConfiguration('linear_turbo_scale'),
            'angular_scale': LaunchConfiguration('angular_scale'),
            'angular_turbo_scale': LaunchConfiguration('angular_turbo_scale'),
            'active_joint': LaunchConfiguration('active_joint'),
            'device_id': LaunchConfiguration('device_id'),
        }.items(),
    )

    # YDLidar TG driver — vendor LifecycleNode, named to match the
    # vendor params yaml (which keys on `ydlidar_ros2_driver_node`).
    ydlidar_node = LifecycleNode(
        package='ydlidar_ros2_driver',
        executable='ydlidar_ros2_driver_node',
        name='ydlidar_ros2_driver_node',
        namespace='/',
        output='screen',
        emulate_tty=True,
        parameters=[LaunchConfiguration('ydlidar_params')],
        condition=IfCondition(LaunchConfiguration('enable_lidar')),
    )

    # Auto-configure on launch start: UNCONFIGURED → INACTIVE.
    configure_ydlidar = EmitEvent(
        event=ChangeState(
            lifecycle_node_matcher=lambda action: action.node_name == 'ydlidar_ros2_driver_node',
            transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
        ),
        condition=IfCondition(LaunchConfiguration('enable_lidar')),
    )

    # Auto-activate when configure completes: INACTIVE → ACTIVE.
    # Without this, the node publishes nothing because the lifecycle
    # state machine sits in INACTIVE.
    activate_ydlidar_on_configure = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=ydlidar_node,
            start_state='configuring',
            goal_state='inactive',
            entities=[
                LogInfo(msg='[master.launch] ydlidar configured → activating'),
                EmitEvent(event=ChangeState(
                    lifecycle_node_matcher=lambda action: action.node_name == 'ydlidar_ros2_driver_node',
                    transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
                )),
            ],
        ),
    )

    # Static TF: base_link → laser. Vendor 4ROS branch values
    # (laser_bringup_no_odom_launch.py): xyz=(0.0435, 5.258E-05, 0.11),
    # no rotation. RPLidar variants use rpy=(3.14,0,0) — do NOT use
    # those here.
    laser_static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_pub_laser',
        arguments=[
            '0.0435', '5.258E-05', '0.11',  # xyz
            '0', '0', '0',                  # rpy
            'base_link', 'laser',
        ],
        condition=IfCondition(LaunchConfiguration('enable_lidar')),
    )

    return LaunchDescription([
        arm_jog_rate_arg,
        linear_scale_arg,
        linear_turbo_scale_arg,
        angular_scale_arg,
        angular_turbo_scale_arg,
        active_joint_arg,
        device_id_arg,
        enable_lidar_arg,
        ydlidar_params_arg,
        pre_flight_banner,
        phase5_combined,
        ydlidar_node,
        configure_ydlidar,
        activate_ydlidar_on_configure,
        laser_static_tf,
    ])
