import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/jetson/yahboomcar_ros2_ws_new/yahboomcar_ws/install/yahboomcar_rviz'
