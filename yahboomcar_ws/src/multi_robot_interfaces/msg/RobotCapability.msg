# A single capability that a robot hardware provides
string robot_name          # "rosmaster" or "ultra"
string capability_name     # "arm", "gripper", "camera_rgb", "camera_depth", "mobile_base", "lidar"
string status              # "ready", "busy", "error", "offline"
string[] supported_actions # e.g. ["move_to_pose", "detect_color", "track_object"]
string hardware_info       # e.g. "6DOF_MoveIt2" or "6DOF_servo_bus"
