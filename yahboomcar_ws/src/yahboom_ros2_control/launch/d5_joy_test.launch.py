"""D5 Phase 1 — bring up just joy_node so joy_recorder.py can subscribe to /joy.

joy_node (ros-humble-joy v3.3.0) uses `device_id` (int) to pick which
/dev/input/jsN to read. Default 0 = /dev/input/js0. NOT a path param —
our /dev/yahboom_joy udev symlink isn't directly used here, but it IS
the symlink target of /dev/input/js0 on the host (verified D1.5 power
cycle). Inside the yahboom_ros2_humble container, /dev/input is bind
mounted; if /dev/input/js0 is missing inside the container, restart
the container to refresh the snapshot:

    docker restart yahboom_ros2_humble

Usage:
  Terminal A (container):  ros2 launch yahboom_ros2_control d5_joy_test.launch.py
  Terminal B (container):  ros2 run yahboom_ros2_control joy_recorder.py

Override the device index if you have multiple joysticks plugged in:
  ros2 launch yahboom_ros2_control d5_joy_test.launch.py device_id:=1
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    device_id_arg = DeclareLaunchArgument(
        'device_id', default_value='0',
        description='Joystick index. 0 = /dev/input/js0 (our udev /dev/yahboom_joy target).',
    )
    rate_arg = DeclareLaunchArgument(
        'autorepeat_rate', default_value='50.0',
        description='Hz. Joy publishes at this rate even when nothing changes.',
    )

    joy = Node(
        package='joy',
        executable='joy_node',
        name='joy_node',
        output='screen',
        parameters=[{
            'device_id': LaunchConfiguration('device_id'),
            'deadzone': 0.0,            # raw values for D5 mapping verification
            'autorepeat_rate': LaunchConfiguration('autorepeat_rate'),
            'sticky_buttons': False,
            'coalesce_interval_ms': 1,
        }],
    )

    return LaunchDescription([device_id_arg, rate_arg, joy])
