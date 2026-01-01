// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from yahboomcar_msgs:msg/TargetArray.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_HPP_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'data'
#include "yahboomcar_msgs/msg/detail/target__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__yahboomcar_msgs__msg__TargetArray __attribute__((deprecated))
#else
# define DEPRECATED__yahboomcar_msgs__msg__TargetArray __declspec(deprecated)
#endif

namespace yahboomcar_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct TargetArray_
{
  using Type = TargetArray_<ContainerAllocator>;

  explicit TargetArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit TargetArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _data_type =
    std::vector<yahboomcar_msgs::msg::Target_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<yahboomcar_msgs::msg::Target_<ContainerAllocator>>>;
  _data_type data;

  // setters for named parameter idiom
  Type & set__data(
    const std::vector<yahboomcar_msgs::msg::Target_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<yahboomcar_msgs::msg::Target_<ContainerAllocator>>> & _arg)
  {
    this->data = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__yahboomcar_msgs__msg__TargetArray
    std::shared_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__yahboomcar_msgs__msg__TargetArray
    std::shared_ptr<yahboomcar_msgs::msg::TargetArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TargetArray_ & other) const
  {
    if (this->data != other.data) {
      return false;
    }
    return true;
  }
  bool operator!=(const TargetArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TargetArray_

// alias to use template instance with default allocator
using TargetArray =
  yahboomcar_msgs::msg::TargetArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace yahboomcar_msgs

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_HPP_
