// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from yahboomcar_msgs:msg/ImageMsg.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__TRAITS_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "yahboomcar_msgs/msg/detail/image_msg__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace yahboomcar_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const ImageMsg & msg,
  std::ostream & out)
{
  out << "{";
  // member: height
  {
    out << "height: ";
    rosidl_generator_traits::value_to_yaml(msg.height, out);
    out << ", ";
  }

  // member: width
  {
    out << "width: ";
    rosidl_generator_traits::value_to_yaml(msg.width, out);
    out << ", ";
  }

  // member: channels
  {
    out << "channels: ";
    rosidl_generator_traits::value_to_yaml(msg.channels, out);
    out << ", ";
  }

  // member: data
  {
    if (msg.data.size() == 0) {
      out << "data: []";
    } else {
      out << "data: [";
      size_t pending_items = msg.data.size();
      for (auto item : msg.data) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ImageMsg & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: height
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "height: ";
    rosidl_generator_traits::value_to_yaml(msg.height, out);
    out << "\n";
  }

  // member: width
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "width: ";
    rosidl_generator_traits::value_to_yaml(msg.width, out);
    out << "\n";
  }

  // member: channels
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "channels: ";
    rosidl_generator_traits::value_to_yaml(msg.channels, out);
    out << "\n";
  }

  // member: data
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.data.size() == 0) {
      out << "data: []\n";
    } else {
      out << "data:\n";
      for (auto item : msg.data) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ImageMsg & msg, bool use_flow_style = false)
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
  const yahboomcar_msgs::msg::ImageMsg & msg,
  std::ostream & out, size_t indentation = 0)
{
  yahboomcar_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use yahboomcar_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const yahboomcar_msgs::msg::ImageMsg & msg)
{
  return yahboomcar_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<yahboomcar_msgs::msg::ImageMsg>()
{
  return "yahboomcar_msgs::msg::ImageMsg";
}

template<>
inline const char * name<yahboomcar_msgs::msg::ImageMsg>()
{
  return "yahboomcar_msgs/msg/ImageMsg";
}

template<>
struct has_fixed_size<yahboomcar_msgs::msg::ImageMsg>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<yahboomcar_msgs::msg::ImageMsg>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<yahboomcar_msgs::msg::ImageMsg>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__TRAITS_HPP_
