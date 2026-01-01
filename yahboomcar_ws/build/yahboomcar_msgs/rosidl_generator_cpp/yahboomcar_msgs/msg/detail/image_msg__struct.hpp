// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboomcar_msgs:msg/ImageMsg.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__yahboomcar_msgs__msg__ImageMsg __attribute__((deprecated))
#else
# define DEPRECATED__yahboomcar_msgs__msg__ImageMsg __declspec(deprecated)
#endif

namespace yahboomcar_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ImageMsg_
{
  using Type = ImageMsg_<ContainerAllocator>;

  explicit ImageMsg_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->height = 0l;
      this->width = 0l;
      this->channels = 0l;
    }
  }

  explicit ImageMsg_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->height = 0l;
      this->width = 0l;
      this->channels = 0l;
    }
  }

  // field types and members
  using _height_type =
    int32_t;
  _height_type height;
  using _width_type =
    int32_t;
  _width_type width;
  using _channels_type =
    int32_t;
  _channels_type channels;
  using _data_type =
    std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>>;
  _data_type data;

  // setters for named parameter idiom
  Type & set__height(
    const int32_t & _arg)
  {
    this->height = _arg;
    return *this;
  }
  Type & set__width(
    const int32_t & _arg)
  {
    this->width = _arg;
    return *this;
  }
  Type & set__channels(
    const int32_t & _arg)
  {
    this->channels = _arg;
    return *this;
  }
  Type & set__data(
    const std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>> & _arg)
  {
    this->data = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboomcar_msgs__msg__ImageMsg
    std::shared_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboomcar_msgs__msg__ImageMsg
    std::shared_ptr<yahboomcar_msgs::msg::ImageMsg_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ImageMsg_ & other) const
  {
    if (this->height != other.height) {
      return false;
    }
    if (this->width != other.width) {
      return false;
    }
    if (this->channels != other.channels) {
      return false;
    }
    if (this->data != other.data) {
      return false;
    }
    return true;
  }
  bool operator!=(const ImageMsg_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ImageMsg_

// alias to use template instance with default allocator
using ImageMsg =
  yahboomcar_msgs::msg::ImageMsg_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__IMAGE_MSG__STRUCT_HPP_
