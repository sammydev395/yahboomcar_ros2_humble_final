// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboomcar_msgs:msg/Target.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__STRUCT_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__yahboomcar_msgs__msg__Target __attribute__((deprecated))
#else
# define DEPRECATED__yahboomcar_msgs__msg__Target __declspec(deprecated)
#endif

namespace yahboomcar_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Target_
{
  using Type = Target_<ContainerAllocator>;

  explicit Target_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->frame_id = "";
      this->scores = 0.0f;
      this->ptx = 0.0f;
      this->pty = 0.0f;
      this->distw = 0.0f;
      this->disth = 0.0f;
      this->centerx = 0.0f;
      this->centery = 0.0f;
    }
  }

  explicit Target_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : frame_id(_alloc),
    stamp(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->frame_id = "";
      this->scores = 0.0f;
      this->ptx = 0.0f;
      this->pty = 0.0f;
      this->distw = 0.0f;
      this->disth = 0.0f;
      this->centerx = 0.0f;
      this->centery = 0.0f;
    }
  }

  // field types and members
  using _frame_id_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _frame_id_type frame_id;
  using _stamp_type =
    builtin_interfaces::msg::Time_<ContainerAllocator>;
  _stamp_type stamp;
  using _scores_type =
    float;
  _scores_type scores;
  using _ptx_type =
    float;
  _ptx_type ptx;
  using _pty_type =
    float;
  _pty_type pty;
  using _distw_type =
    float;
  _distw_type distw;
  using _disth_type =
    float;
  _disth_type disth;
  using _centerx_type =
    float;
  _centerx_type centerx;
  using _centery_type =
    float;
  _centery_type centery;

  // setters for named parameter idiom
  Type & set__frame_id(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->frame_id = _arg;
    return *this;
  }
  Type & set__stamp(
    const builtin_interfaces::msg::Time_<ContainerAllocator> & _arg)
  {
    this->stamp = _arg;
    return *this;
  }
  Type & set__scores(
    const float & _arg)
  {
    this->scores = _arg;
    return *this;
  }
  Type & set__ptx(
    const float & _arg)
  {
    this->ptx = _arg;
    return *this;
  }
  Type & set__pty(
    const float & _arg)
  {
    this->pty = _arg;
    return *this;
  }
  Type & set__distw(
    const float & _arg)
  {
    this->distw = _arg;
    return *this;
  }
  Type & set__disth(
    const float & _arg)
  {
    this->disth = _arg;
    return *this;
  }
  Type & set__centerx(
    const float & _arg)
  {
    this->centerx = _arg;
    return *this;
  }
  Type & set__centery(
    const float & _arg)
  {
    this->centery = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboomcar_msgs::msg::Target_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboomcar_msgs::msg::Target_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::Target_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::Target_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboomcar_msgs__msg__Target
    std::shared_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboomcar_msgs__msg__Target
    std::shared_ptr<yahboomcar_msgs::msg::Target_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Target_ & other) const
  {
    if (this->frame_id != other.frame_id) {
      return false;
    }
    if (this->stamp != other.stamp) {
      return false;
    }
    if (this->scores != other.scores) {
      return false;
    }
    if (this->ptx != other.ptx) {
      return false;
    }
    if (this->pty != other.pty) {
      return false;
    }
    if (this->distw != other.distw) {
      return false;
    }
    if (this->disth != other.disth) {
      return false;
    }
    if (this->centerx != other.centerx) {
      return false;
    }
    if (this->centery != other.centery) {
      return false;
    }
    return true;
  }
  bool operator!=(const Target_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Target_

// alias to use template instance with default allocator
using Target =
  yahboomcar_msgs::msg::Target_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET__STRUCT_HPP_
