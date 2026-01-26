# yahboomcar_msgs Package Catalog

---

## Overview
**Path:** `src/yahboomcar_msgs/`

Defines custom ROS messages and services for Yahboom ROSMASTER robots, supporting communication for arm control, perception, and target tracking.

---

## Message Definitions (`src/yahboomcar_msgs/msg/`)

### src/yahboomcar_msgs/msg/ArmJoint.msg
**Fields:**
- `int32 id`
- `int32 run_time`
- `float32 angle`
- `float32[] joints`
**Use:** For controlling or reporting arm joint states.

---

### src/yahboomcar_msgs/msg/Image_Msg.msg
**Fields:**
- `int32 height`
- `int32 width`
- `int32 channels`
- `uint8[] data`
**Use:** For transmitting image data in a custom format.

---

### src/yahboomcar_msgs/msg/PointArray.msg
**Fields:**
- `geometry_msgs/Point[] points`
**Use:** For sending arrays of 3D points.

---

### src/yahboomcar_msgs/msg/Position.msg
**Fields:**
- `float32 angleX`
- `float32 angleY`
- `float32 distance`
**Use:** For reporting detected object positions.

---

### src/yahboomcar_msgs/msg/Target.msg
**Fields:**
- `string frame_id`
- `time stamp`
- `float32 scores`
- `float32 ptx, pty, distw, disth, centerx, centery`
**Use:** For describing detected targets with scores and positions.

---

### src/yahboomcar_msgs/msg/TargetArray.msg
**Fields:**
- `yahboomcar_msgs/Target[] data`
**Use:** For sending arrays of detected targets.

---

## Service Definitions (`src/yahboomcar_msgs/srv/`)

### src/yahboomcar_msgs/srv/RobotArmArray.srv
**Request:**
- `string apply`
**Response:**
- `float64[] angles`
**Use:** For requesting or commanding arm joint angles.

---

### src/yahboomcar_msgs/srv/kinemarics.srv
**Request:**
- `string kin_name`
- `float64[] src_pos`
- `float64[] src_joints`
**Response:**
- `float64[] target_joints`
- `float64[] target_pos`
**Use:** For performing forward/inverse kinematics calculations.

---

## Dependencies
- `actionlib_msgs`, `geometry_msgs`, `message_generation`, `message_runtime`

---

**This package is required for all Yahboom robot nodes that communicate custom arm, perception, or target-tracking data.** 