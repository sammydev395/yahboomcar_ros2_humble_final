// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_HPP_
#define YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Request __attribute__((deprecated))
#else
# define DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Request __declspec(deprecated)
#endif

namespace yahboom_web_savmap_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct WebSaveMap_Request_
{
  using Type = WebSaveMap_Request_<ContainerAllocator>;

  explicit WebSaveMap_Request_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->mapname = "";
    }
  }

  explicit WebSaveMap_Request_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : mapname(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->mapname = "";
    }
  }

  // field types and members
  using _mapname_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _mapname_type mapname;

  // setters for named parameter idiom
  Type & set__mapname(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->mapname = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Request
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Request
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const WebSaveMap_Request_ & other) const
  {
    if (this->mapname != other.mapname) {
      return false;
    }
    return true;
  }
  bool operator!=(const WebSaveMap_Request_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct WebSaveMap_Request_

// alias to use template instance with default allocator
using WebSaveMap_Request =
  yahboom_web_savmap_interfaces::srv::WebSaveMap_Request_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace yahboom_web_savmap_interfaces


#ifndef _WIN32
# define DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Response __attribute__((deprecated))
#else
# define DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Response __declspec(deprecated)
#endif

namespace yahboom_web_savmap_interfaces
{

namespace srv
{

// message struct
template<class ContainerAllocator>
struct WebSaveMap_Response_
{
  using Type = WebSaveMap_Response_<ContainerAllocator>;

  explicit WebSaveMap_Response_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->response = "";
    }
  }

  explicit WebSaveMap_Response_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : response(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->response = "";
    }
  }

  // field types and members
  using _response_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _response_type response;

  // setters for named parameter idiom
  Type & set__response(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->response = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Response
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboom_web_savmap_interfaces__srv__WebSaveMap_Response
    std::shared_ptr<yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const WebSaveMap_Response_ & other) const
  {
    if (this->response != other.response) {
      return false;
    }
    return true;
  }
  bool operator!=(const WebSaveMap_Response_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct WebSaveMap_Response_

// alias to use template instance with default allocator
using WebSaveMap_Response =
  yahboom_web_savmap_interfaces::srv::WebSaveMap_Response_<std::allocator<void>>;

// constant definitions

}  // namespace srv

}  // namespace yahboom_web_savmap_interfaces

namespace yahboom_web_savmap_interfaces
{

namespace srv
{

struct WebSaveMap
{
  using Request = yahboom_web_savmap_interfaces::srv::WebSaveMap_Request;
  using Response = yahboom_web_savmap_interfaces::srv::WebSaveMap_Response;
};

}  // namespace srv

}  // namespace yahboom_web_savmap_interfaces

#endif  // YAHBOOM_WEB_SAVMAP_INTERFACES__SRV__DETAIL__WEB_SAVE_MAP__STRUCT_HPP_
