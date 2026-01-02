from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='yahboomcar_astra',
            executable='astra_camera_node',
            name='astra_camera',
            output='screen',
            parameters=[{
                "enable_color": True,
                "use_uvc_camera": True,
                "color_width": 640,
                "color_height": 480,
                "color_fps": 30,
                "uvc_camera_format": "RGB",
                "uvc_vendor_id": 0x2bc5,
                "uvc_product_id": 0x050f,
                "enable_depth": True,
                "enable_ir": True,
                "enable_pointcloud": True
            }],
            arguments=['--ros-args', '--log-level', 'warn']
        )
    ])
