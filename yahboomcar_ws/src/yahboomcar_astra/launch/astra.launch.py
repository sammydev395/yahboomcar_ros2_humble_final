from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='yahboomcar_astra',
            executable='astra_camera_node',
            name='astra_camera',
            # Namespaced 2026-09-26: bare /color, /depth, /ir are
            # fleet-global on ROS_DOMAIN_ID=100. Topics are now
            # /rosmaster/{color,depth,ir}/... Vendor demo nodes
            # (colorTracker, colorHSV, rtabmap, mediapipe, visual) still
            # subscribe to the bare names; run them in the rosmaster
            # namespace (--ros-args -r __ns:=/rosmaster) or remap.
            namespace='rosmaster',
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
