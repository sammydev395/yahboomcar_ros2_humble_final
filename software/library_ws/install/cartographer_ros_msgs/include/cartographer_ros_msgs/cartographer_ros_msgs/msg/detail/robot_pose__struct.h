// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from cartographer_ros_msgs:msg/RobotPose.idl
// generated code does not contain a copyright notice

#ifndef CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_H_
#define CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'robot_pose'
// Member 'last_update_pose'
#include "geometry_msgs/msg/detail/pose__struct.h"
// Member 'current_trajectory'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/RobotPose in the package cartographer_ros_msgs.
/**
  * Copyright 2018 The Cartographer Authors
  *
  * Licensed under the Apache License, Version 2.0 (the 'License');
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *      http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an 'AS IS' BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
 */
typedef struct cartographer_ros_msgs__msg__RobotPose
{
  geometry_msgs__msg__Pose robot_pose;
  float covariance_score;
  rosidl_runtime_c__String current_trajectory;
  geometry_msgs__msg__Pose last_update_pose;
  float last_update_duration;
} cartographer_ros_msgs__msg__RobotPose;

// Struct for a sequence of cartographer_ros_msgs__msg__RobotPose.
typedef struct cartographer_ros_msgs__msg__RobotPose__Sequence
{
  cartographer_ros_msgs__msg__RobotPose * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} cartographer_ros_msgs__msg__RobotPose__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_H_
