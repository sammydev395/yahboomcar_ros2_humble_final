// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from yahboomcar_msgs:msg/TargetArray.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__BUILDER_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "yahboomcar_msgs/msg/detail/target_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace yahboomcar_msgs
{

namespace msg
{

namespace builder
{

class Init_TargetArray_data
{
public:
  Init_TargetArray_data()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::yahboomcar_msgs::msg::TargetArray data(::yahboomcar_msgs::msg::TargetArray::_data_type arg)
  {
    msg_.data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboomcar_msgs::msg::TargetArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboomcar_msgs::msg::TargetArray>()
{
  return yahboomcar_msgs::msg::builder::Init_TargetArray_data();
}

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__BUILDER_HPP_
