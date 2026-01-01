// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__BUILDER_HPP_
#define YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "yahboom_web_savmap_interfaces/srv/detail/web_save_map__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace yahboom_web_savmap_interfaces
{

namespace srv
{

namespace builder
{

class Init_WebSaveMap_Request_mapname
{
public:
  Init_WebSaveMap_Request_mapname()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::yahboom_web_savmap_interfaces::srv::WebSaveMap_Request mapname(::yahboom_web_savmap_interfaces::srv::WebSaveMap_Request::_mapname_type arg)
  {
    msg_.mapname = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboom_web_savmap_interfaces::srv::WebSaveMap_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboom_web_savmap_interfaces::srv::WebSaveMap_Request>()
{
  return yahboom_web_savmap_interfaces::srv::builder::Init_WebSaveMap_Request_mapname();
}

}  // namespace yahboom_web_savmap_interfaces


namespace yahboom_web_savmap_interfaces
{

namespace srv
{

namespace builder
{

class Init_WebSaveMap_Response_response
{
public:
  Init_WebSaveMap_Response_response()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::yahboom_web_savmap_interfaces::srv::WebSaveMap_Response response(::yahboom_web_savmap_interfaces::srv::WebSaveMap_Response::_response_type arg)
  {
    msg_.response = std::move(arg);
    return std::move(msg_);
  }

private:
  ::yahboom_web_savmap_interfaces::srv::WebSaveMap_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::yahboom_web_savmap_interfaces::srv::WebSaveMap_Response>()
{
  return yahboom_web_savmap_interfaces::srv::builder::Init_WebSaveMap_Response_response();
}

}  // namespace yahboom_web_savmap_interfaces

#endif  // YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__BUILDER_HPP_
