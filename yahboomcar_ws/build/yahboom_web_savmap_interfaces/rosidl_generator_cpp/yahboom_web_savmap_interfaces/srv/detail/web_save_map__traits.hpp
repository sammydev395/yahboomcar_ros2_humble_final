// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__TRAITS_HPP_
#define YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "yahboom_web_savmap_interfaces/srv/detail/web_save_map__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace yahboom_web_savmap_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const WebSaveMap_Request & msg,
  std::ostream & out)
{
  out << "{";
  // member: mapname
  {
    out << "mapname: ";
    rosidl_generator_traits::value_to_yaml(msg.mapname, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const WebSaveMap_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: mapname
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "mapname: ";
    rosidl_generator_traits::value_to_yaml(msg.mapname, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const WebSaveMap_Request & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace yahboom_web_savmap_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use yahboom_web_savmap_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const yahboom_web_savmap_interfaces::srv::WebSaveMap_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  yahboom_web_savmap_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use yahboom_web_savmap_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const yahboom_web_savmap_interfaces::srv::WebSaveMap_Request & msg)
{
  return yahboom_web_savmap_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>()
{
  return "yahboom_web_savmap_interfaces::srv::WebSaveMap_Request";
}

template<>
inline const char * name<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>()
{
  return "yahboom_web_savmap_interfaces/srv/WebSaveMap_Request";
}

template<>
struct has_fixed_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace yahboom_web_savmap_interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const WebSaveMap_Response & msg,
  std::ostream & out)
{
  out << "{";
  // member: response
  {
    out << "response: ";
    rosidl_generator_traits::value_to_yaml(msg.response, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const WebSaveMap_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: response
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "response: ";
    rosidl_generator_traits::value_to_yaml(msg.response, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const WebSaveMap_Response & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace yahboom_web_savmap_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use yahboom_web_savmap_interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const yahboom_web_savmap_interfaces::srv::WebSaveMap_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  yahboom_web_savmap_interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use yahboom_web_savmap_interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const yahboom_web_savmap_interfaces::srv::WebSaveMap_Response & msg)
{
  return yahboom_web_savmap_interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>()
{
  return "yahboom_web_savmap_interfaces::srv::WebSaveMap_Response";
}

template<>
inline const char * name<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>()
{
  return "yahboom_web_savmap_interfaces/srv/WebSaveMap_Response";
}

template<>
struct has_fixed_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<yahboom_web_savmap_interfaces::srv::WebSaveMap>()
{
  return "yahboom_web_savmap_interfaces::srv::WebSaveMap";
}

template<>
inline const char * name<yahboom_web_savmap_interfaces::srv::WebSaveMap>()
{
  return "yahboom_web_savmap_interfaces/srv/WebSaveMap";
}

template<>
struct has_fixed_size<yahboom_web_savmap_interfaces::srv::WebSaveMap>
  : std::integral_constant<
    bool,
    has_fixed_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>::value &&
    has_fixed_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>::value
  >
{
};

template<>
struct has_bounded_size<yahboom_web_savmap_interfaces::srv::WebSaveMap>
  : std::integral_constant<
    bool,
    has_bounded_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>::value &&
    has_bounded_size<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>::value
  >
{
};

template<>
struct is_service<yahboom_web_savmap_interfaces::srv::WebSaveMap>
  : std::true_type
{
};

template<>
struct is_service_request<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>
  : std::true_type
{
};

template<>
struct is_service_response<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>
  : std::true_type
{
};

}  // namespace rosidl_generator_traits

#endif  // YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__TRAITS_HPP_
