// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from cartographer_ros_msgs:msg/RobotPose.idl
// generated code does not contain a copyright notice

#ifndef CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__TRAITS_HPP_
#define CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "cartographer_ros_msgs/msg/detail/robot_pose__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'robot_pose'
// Member 'last_update_pose'
#include "geometry_msgs/msg/detail/pose__traits.hpp"

namespace cartographer_ros_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const RobotPose & msg,
  std::ostream & out)
{
  out << "{";
  // member: robot_pose
  {
    out << "robot_pose: ";
    to_flow_style_yaml(msg.robot_pose, out);
    out << ", ";
  }

  // member: covariance_score
  {
    out << "covariance_score: ";
    rosidl_generator_traits::value_to_yaml(msg.covariance_score, out);
    out << ", ";
  }

  // member: current_trajectory
  {
    out << "current_trajectory: ";
    rosidl_generator_traits::value_to_yaml(msg.current_trajectory, out);
    out << ", ";
  }

  // member: last_update_pose
  {
    out << "last_update_pose: ";
    to_flow_style_yaml(msg.last_update_pose, out);
    out << ", ";
  }

  // member: last_update_duration
  {
    out << "last_update_duration: ";
    rosidl_generator_traits::value_to_yaml(msg.last_update_duration, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const RobotPose & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: robot_pose
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "robot_pose:\n";
    to_block_style_yaml(msg.robot_pose, out, indentation + 2);
  }

  // member: covariance_score
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "covariance_score: ";
    rosidl_generator_traits::value_to_yaml(msg.covariance_score, out);
    out << "\n";
  }

  // member: current_trajectory
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "current_trajectory: ";
    rosidl_generator_traits::value_to_yaml(msg.current_trajectory, out);
    out << "\n";
  }

  // member: last_update_pose
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "last_update_pose:\n";
    to_block_style_yaml(msg.last_update_pose, out, indentation + 2);
  }

  // member: last_update_duration
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "last_update_duration: ";
    rosidl_generator_traits::value_to_yaml(msg.last_update_duration, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const RobotPose & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace cartographer_ros_msgs

namespace rosidl_generator_traits
{

[[deprecated("use cartographer_ros_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const cartographer_ros_msgs::msg::RobotPose & msg,
  std::ostream & out, size_t indentation = 0)
{
  cartographer_ros_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use cartographer_ros_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const cartographer_ros_msgs::msg::RobotPose & msg)
{
  return cartographer_ros_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<cartographer_ros_msgs::msg::RobotPose>()
{
  return "cartographer_ros_msgs::msg::RobotPose";
}

template<>
inline const char * name<cartographer_ros_msgs::msg::RobotPose>()
{
  return "cartographer_ros_msgs/msg/RobotPose";
}

template<>
struct has_fixed_size<cartographer_ros_msgs::msg::RobotPose>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<cartographer_ros_msgs::msg::RobotPose>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<cartographer_ros_msgs::msg::RobotPose>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__TRAITS_HPP_
