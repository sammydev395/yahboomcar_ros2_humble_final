// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from yahboomcar_msgs:msg/Target.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__BUILDER_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "yahboomcar_msgs/msg/detail/target__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace yahboomcar_msgs
{

namespace msg
{

namespace builder
{

class Init_Target_centery
{
public:
  explicit Init_Target_centery(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  ::yahboomcar_msgs::msg::Target centery(::yahboomcar_msgs::msg::Target::_centery_type arg)
  {
    msg_.centery = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_centerx
{
public:
  explicit Init_Target_centerx(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_centery centerx(::yahboomcar_msgs::msg::Target::_centerx_type arg)
  {
    msg_.centerx = std::move(arg);
    return Init_Target_centery(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_disth
{
public:
  explicit Init_Target_disth(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_centerx disth(::yahboomcar_msgs::msg::Target::_disth_type arg)
  {
    msg_.disth = std::move(arg);
    return Init_Target_centerx(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_distw
{
public:
  explicit Init_Target_distw(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_disth distw(::yahboomcar_msgs::msg::Target::_distw_type arg)
  {
    msg_.distw = std::move(arg);
    return Init_Target_disth(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_pty
{
public:
  explicit Init_Target_pty(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_distw pty(::yahboomcar_msgs::msg::Target::_pty_type arg)
  {
    msg_.pty = std::move(arg);
    return Init_Target_distw(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_ptx
{
public:
  explicit Init_Target_ptx(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_pty ptx(::yahboomcar_msgs::msg::Target::_ptx_type arg)
  {
    msg_.ptx = std::move(arg);
    return Init_Target_pty(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_scores
{
public:
  explicit Init_Target_scores(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_ptx scores(::yahboomcar_msgs::msg::Target::_scores_type arg)
  {
    msg_.scores = std::move(arg);
    return Init_Target_ptx(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_stamp
{
public:
  explicit Init_Target_stamp(::yahboomcar_msgs::msg::Target & msg)
  : msg_(msg)
  {}
  Init_Target_scores stamp(::yahboomcar_msgs::msg::Target::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return Init_Target_scores(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

class Init_Target_frame_id
{
public:
  Init_Target_frame_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Target_stamp frame_id(::yahboomcar_msgs::msg::Target::_frame_id_type arg)
  {
    msg_.frame_id = std::move(arg);
    return Init_Target_stamp(msg_);
  }

private:
  ::yahboomcar_msgs::msg::Target msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboomcar_msgs::msg::Target>()
{
  return yahboomcar_msgs::msg::builder::Init_Target_frame_id();
}

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__BUILDER_HPP_
