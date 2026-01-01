// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from yahboomcar_msgs:msg/PointArray.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__POINT_ARRAY__BUILDER_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__POINT_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "yahboomcar_msgs/msg/detail/point_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace yahboomcar_msgs
{

namespace msg
{

namespace builder
{

class Init_PointArray_points
{
public:
  Init_PointArray_points()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::yahboomcar_msgs::msg::PointArray points(::yahboomcar_msgs::msg::PointArray::_points_type arg)
  {
    msg_.points = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboomcar_msgs::msg::PointArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboomcar_msgs::msg::PointArray>()
{
  return yahboomcar_msgs::msg::builder::Init_PointArray_points();
}

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__POINT_ARRAY__BUILDER_HPP_
