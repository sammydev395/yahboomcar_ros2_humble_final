// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from yahboomcar_msgs:msg/Target.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__TRAITS_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "yahboomcar_msgs/msg/detail/target__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__traits.hpp"

namespace yahboomcar_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const Target & msg,
  std::ostream & out)
{
  out << "{";
  // member: frame_id
  {
    out << "frame_id: ";
    rosidl_generator_traits::value_to_yaml(msg.frame_id, out);
    out << ", ";
  }

  // member: stamp
  {
    out << "stamp: ";
    to_flow_style_yaml(msg.stamp, out);
    out << ", ";
  }

  // member: scores
  {
    out << "scores: ";
    rosidl_generator_traits::value_to_yaml(msg.scores, out);
    out << ", ";
  }

  // member: ptx
  {
    out << "ptx: ";
    rosidl_generator_traits::value_to_yaml(msg.ptx, out);
    out << ", ";
  }

  // member: pty
  {
    out << "pty: ";
    rosidl_generator_traits::value_to_yaml(msg.pty, out);
    out << ", ";
  }

  // member: distw
  {
    out << "distw: ";
    rosidl_generator_traits::value_to_yaml(msg.distw, out);
    out << ", ";
  }

  // member: disth
  {
    out << "disth: ";
    rosidl_generator_traits::value_to_yaml(msg.disth, out);
    out << ", ";
  }

  // member: centerx
  {
    out << "centerx: ";
    rosidl_generator_traits::value_to_yaml(msg.centerx, out);
    out << ", ";
  }

  // member: centery
  {
    out << "centery: ";
    rosidl_generator_traits::value_to_yaml(msg.centery, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Target & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: frame_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "frame_id: ";
    rosidl_generator_traits::value_to_yaml(msg.frame_id, out);
    out << "\n";
  }

  // member: stamp
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "stamp:\n";
    to_block_style_yaml(msg.stamp, out, indentation + 2);
  }

  // member: scores
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "scores: ";
    rosidl_generator_traits::value_to_yaml(msg.scores, out);
    out << "\n";
  }

  // member: ptx
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "ptx: ";
    rosidl_generator_traits::value_to_yaml(msg.ptx, out);
    out << "\n";
  }

  // member: pty
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pty: ";
    rosidl_generator_traits::value_to_yaml(msg.pty, out);
    out << "\n";
  }

  // member: distw
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "distw: ";
    rosidl_generator_traits::value_to_yaml(msg.distw, out);
    out << "\n";
  }

  // member: disth
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "disth: ";
    rosidl_generator_traits::value_to_yaml(msg.disth, out);
    out << "\n";
  }

  // member: centerx
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "centerx: ";
    rosidl_generator_traits::value_to_yaml(msg.centerx, out);
    out << "\n";
  }

  // member: centery
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "centery: ";
    rosidl_generator_traits::value_to_yaml(msg.centery, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Target & msg, bool use_flow_style = false)
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

}  // namespace yahboomcar_msgs

namespace rosidl_generator_traits
{

[[deprecated("use yahboomcar_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const yahboomcar_msgs::msg::Target & msg,
  std::ostream & out, size_t indentation = 0)
{
  yahboomcar_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use yahboomcar_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const yahboomcar_msgs::msg::Target & msg)
{
  return yahboomcar_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<yahboomcar_msgs::msg::Target>()
{
  return "yahboomcar_msgs::msg::Target";
}

template<>
inline const char * name<yahboomcar_msgs::msg::Target>()
{
  return "yahboomcar_msgs/msg/Target";
}

template<>
struct has_fixed_size<yahboomcar_msgs::msg::Target>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<yahboomcar_msgs::msg::Target>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<yahboomcar_msgs::msg::Target>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__TRAITS_HPP_
