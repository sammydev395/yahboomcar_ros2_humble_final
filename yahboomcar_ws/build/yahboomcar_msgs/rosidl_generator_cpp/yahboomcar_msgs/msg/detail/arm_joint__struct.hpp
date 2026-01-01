// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboomcar_msgs:msg/ArmJoint.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__STRUCT_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__yahboomcar_msgs__msg__ArmJoint __attribute__((deprecated))
#else
# define DEPRECATED__yahboomcar_msgs__msg__ArmJoint __declspec(deprecated)
#endif

namespace yahboomcar_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ArmJoint_
{
  using Type = ArmJoint_<ContainerAllocator>;

  explicit ArmJoint_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->run_time = 0l;
      this->angle = 0.0f;
    }
  }

  explicit ArmJoint_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->run_time = 0l;
      this->angle = 0.0f;
    }
  }

  // field types and members
  using _id_type =
    int32_t;
  _id_type id;
  using _run_time_type =
    int32_t;
  _run_time_type run_time;
  using _angle_type =
    float;
  _angle_type angle;
  using _joints_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _joints_type joints;

  // setters for named parameter idiom
  Type & set__id(
    const int32_t & _arg)
  {
    this->id = _arg;
    return *this;
  }
  Type & set__run_time(
    const int32_t & _arg)
  {
    this->run_time = _arg;
    return *this;
  }
  Type & set__angle(
    const float & _arg)
  {
    this->angle = _arg;
    return *this;
  }
  Type & set__joints(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->joints = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboomcar_msgs__msg__ArmJoint
    std::shared_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboomcar_msgs__msg__ArmJoint
    std::shared_ptr<yahboomcar_msgs::msg::ArmJoint_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ArmJoint_ & other) const
  {
    if (this->id != other.id) {
      return false;
    }
    if (this->run_time != other.run_time) {
      return false;
    }
    if (this->angle != other.angle) {
      return false;
    }
    if (this->joints != other.joints) {
      return false;
    }
    return true;
  }
  bool operator!=(const ArmJoint_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ArmJoint_

// alias to use template instance with default allocator
using ArmJoint =
  yahboomcar_msgs::msg::ArmJoint_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__ARM_JOINT__STRUCT_HPP_
