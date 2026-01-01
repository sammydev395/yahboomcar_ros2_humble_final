// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboomcar_msgs:msg/Position.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__yahboomcar_msgs__msg__Position __attribute__((deprecated))
#else
# define DEPRECATED__yahboomcar_msgs__msg__Position __declspec(deprecated)
#endif

namespace yahboomcar_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Position_
{
  using Type = Position_<ContainerAllocator>;

  explicit Position_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->anglex = 0.0f;
      this->angley = 0.0f;
      this->distance = 0.0f;
    }
  }

  explicit Position_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->anglex = 0.0f;
      this->angley = 0.0f;
      this->distance = 0.0f;
    }
  }

  // field types and members
  using _anglex_type =
    float;
  _anglex_type anglex;
  using _angley_type =
    float;
  _angley_type angley;
  using _distance_type =
    float;
  _distance_type distance;

  // setters for named parameter idiom
  Type & set__anglex(
    const float & _arg)
  {
    this->anglex = _arg;
    return *this;
  }
  Type & set__angley(
    const float & _arg)
  {
    this->angley = _arg;
    return *this;
  }
  Type & set__distance(
    const float & _arg)
  {
    this->distance = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboomcar_msgs::msg::Position_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboomcar_msgs::msg::Position_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::Position_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::Position_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboomcar_msgs__msg__Position
    std::shared_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboomcar_msgs__msg__Position
    std::shared_ptr<yahboomcar_msgs::msg::Position_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Position_ & other) const
  {
    if (this->anglex != other.anglex) {
      return false;
    }
    if (this->angley != other.angley) {
      return false;
    }
    if (this->distance != other.distance) {
      return false;
    }
    return true;
  }
  bool operator!=(const Position_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Position_

// alias to use template instance with default allocator
using Position =
  yahboomcar_msgs::msg::Position_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_HPP_
