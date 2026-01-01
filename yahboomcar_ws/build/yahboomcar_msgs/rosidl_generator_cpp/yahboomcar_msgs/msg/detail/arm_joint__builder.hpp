// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from yahboomcar_msgs:msg/ArmJoint.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__BUILDER_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "yahboomcar_msgs/msg/detail/arm_joint__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace yahboomcar_msgs
{

namespace msg
{

namespace builder
{

class Init_ArmJoint_joints
{
public:
  explicit Init_ArmJoint_joints(::yahboomcar_msgs::msg::ArmJoint & msg)
  : msg_(msg)
  {}
  ::yahboomcar_msgs::msg::ArmJoint joints(::yahboomcar_msgs::msg::ArmJoint::_joints_type arg)
  {
    msg_.joints = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboomcar_msgs::msg::ArmJoint msg_;
};

class Init_ArmJoint_angle
{
public:
  explicit Init_ArmJoint_angle(::yahboomcar_msgs::msg::ArmJoint & msg)
  : msg_(msg)
  {}
  Init_ArmJoint_joints angle(::yahboomcar_msgs::msg::ArmJoint::_angle_type arg)
  {
    msg_.angle = std::move(arg);
    return Init_ArmJoint_joints(msg_);
  }

private:
  ::yahboomcar_msgs::msg::ArmJoint msg_;
};

class Init_ArmJoint_run_time
{
public:
  explicit Init_ArmJoint_run_time(::yahboomcar_msgs::msg::ArmJoint & msg)
  : msg_(msg)
  {}
  Init_ArmJoint_angle run_time(::yahboomcar_msgs::msg::ArmJoint::_run_time_type arg)
  {
    msg_.run_time = std::move(arg);
    return Init_ArmJoint_angle(msg_);
  }

private:
  ::yahboomcar_msgs::msg::ArmJoint msg_;
};

class Init_ArmJoint_id
{
public:
  Init_ArmJoint_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ArmJoint_run_time id(::yahboomcar_msgs::msg::ArmJoint::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_ArmJoint_run_time(msg_);
  }

private:
  ::yahboomcar_msgs::msg::ArmJoint msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboomcar_msgs::msg::ArmJoint>()
{
  return yahboomcar_msgs::msg::builder::Init_ArmJoint_id();
}

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__BUILDER_HPP_
