// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from cartographer_ros_msgs:msg/RobotPose.idl
// generated code does not contain a copyright notice

#ifndef CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__BUILDER_HPP_
#define CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "cartographer_ros_msgs/msg/detail/robot_pose__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace cartographer_ros_msgs
{

namespace msg
{

namespace builder
{

class Init_RobotPose_last_update_duration
{
public:
  explicit Init_RobotPose_last_update_duration(::cartographer_ros_msgs::msg::RobotPose & msg)
  : msg_(msg)
  {}
  ::cartographer_ros_msgs::msg::RobotPose last_update_duration(::cartographer_ros_msgs::msg::RobotPose::_last_update_duration_type arg)
  {
    msg_.last_update_duration = std::move(arg);
    return std::move(msg_);
  }

private:
  ::cartographer_ros_msgs::msg::RobotPose msg_;
};

class Init_RobotPose_last_update_pose
{
public:
  explicit Init_RobotPose_last_update_pose(::cartographer_ros_msgs::msg::RobotPose & msg)
  : msg_(msg)
  {}
  Init_RobotPose_last_update_duration last_update_pose(::cartographer_ros_msgs::msg::RobotPose::_last_update_pose_type arg)
  {
    msg_.last_update_pose = std::move(arg);
    return Init_RobotPose_last_update_duration(msg_);
  }

private:
  ::cartographer_ros_msgs::msg::RobotPose msg_;
};

class Init_RobotPose_current_trajectory
{
public:
  explicit Init_RobotPose_current_trajectory(::cartographer_ros_msgs::msg::RobotPose & msg)
  : msg_(msg)
  {}
  Init_RobotPose_last_update_pose current_trajectory(::cartographer_ros_msgs::msg::RobotPose::_current_trajectory_type arg)
  {
    msg_.current_trajectory = std::move(arg);
    return Init_RobotPose_last_update_pose(msg_);
  }

private:
  ::cartographer_ros_msgs::msg::RobotPose msg_;
};

class Init_RobotPose_covariance_score
{
public:
  explicit Init_RobotPose_covariance_score(::cartographer_ros_msgs::msg::RobotPose & msg)
  : msg_(msg)
  {}
  Init_RobotPose_current_trajectory covariance_score(::cartographer_ros_msgs::msg::RobotPose::_covariance_score_type arg)
  {
    msg_.covariance_score = std::move(arg);
    return Init_RobotPose_current_trajectory(msg_);
  }

private:
  ::cartographer_ros_msgs::msg::RobotPose msg_;
};

class Init_RobotPose_robot_pose
{
public:
  Init_RobotPose_robot_pose()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_RobotPose_covariance_score robot_pose(::cartographer_ros_msgs::msg::RobotPose::_robot_pose_type arg)
  {
    msg_.robot_pose = std::move(arg);
    return Init_RobotPose_covariance_score(msg_);
  }

private:
  ::cartographer_ros_msgs::msg::RobotPose msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::cartographer_ros_msgs::msg::RobotPose>()
{
  return cartographer_ros_msgs::msg::builder::Init_RobotPose_robot_pose();
}

}  // namespace cartographer_ros_msgs

#endif  // CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__BUILDER_HPP_
