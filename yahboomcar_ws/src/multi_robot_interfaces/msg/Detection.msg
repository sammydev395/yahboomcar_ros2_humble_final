# A single object detection result
string label               # "red_cube", "blue_cube", "face", "hand_gesture"
float64 confidence         # 0.0-1.0
float64[3] position_3d     # x,y,z in camera frame (meters); [0,0,0] if unknown
int32[4] bbox_2d           # x,y,w,h in pixels; [0,0,0,0] if unknown
string frame_id            # TF frame of the detection
builtin_interfaces/Time stamp
