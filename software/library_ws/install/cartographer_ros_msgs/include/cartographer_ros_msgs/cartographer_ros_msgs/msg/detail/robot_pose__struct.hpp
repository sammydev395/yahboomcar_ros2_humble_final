// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from cartographer_ros_msgs:msg/RobotPose.idl
// generated code does not contain a copyright notice

#ifndef CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_HPP_
#define CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'robot_pose'
// Member 'last_update_pose'
#include "geometry_msgs/msg/detail/pose__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__cartographer_ros_msgs__msg__RobotPose __attribute__((deprecated))
#else
# define DEPRECATED__cartographer_ros_msgs__msg__RobotPose __declspec(deprecated)
#endif

namespace cartographer_ros_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct RobotPose_
{
  using Type = RobotPose_<ContainerAllocator>;

  explicit RobotPose_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : robot_pose(_init),
    last_update_pose(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->covariance_score = 0.0f;
      this->current_trajectory = "";
      this->last_update_duration = 0.0f;
    }
  }

  explicit RobotPose_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : robot_pose(_alloc, _init),
    current_trajectory(_alloc),
    last_update_pose(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->covariance_score = 0.0f;
      this->current_trajectory = "";
      this->last_update_duration = 0.0f;
    }
  }

  // field types and members
  using _robot_pose_type =
    geometry_msgs::msg::Pose_<ContainerAllocator>;
  _robot_pose_type robot_pose;
  using _covariance_score_type =
    float;
  _covariance_score_type covariance_score;
  using _current_trajectory_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _current_trajectory_type current_trajectory;
  using _last_update_pose_type =
    geometry_msgs::msg::Pose_<ContainerAllocator>;
  _last_update_pose_type last_update_pose;
  using _last_update_duration_type =
    float;
  _last_update_duration_type last_update_duration;

  // setters for named parameter idiom
  Type & set__robot_pose(
    const geometry_msgs::msg::Pose_<ContainerAllocator> & _arg)
  {
    this->robot_pose = _arg;
    return *this;
  }
  Type & set__covariance_score(
    const float & _arg)
  {
    this->covariance_score = _arg;
    return *this;
  }
  Type & set__current_trajectory(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->current_trajectory = _arg;
    return *this;
  }
  Type & set__last_update_pose(
    const geometry_msgs::msg::Pose_<ContainerAllocator> & _arg)
  {
    this->last_update_pose = _arg;
    return *this;
  }
  Type & set__last_update_duration(
    const float & _arg)
  {
    this->last_update_duration = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> *;
  using ConstRawPtr =
    const cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__cartographer_ros_msgs__msg__RobotPose
    std::shared_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__cartographer_ros_msgs__msg__RobotPose
    std::shared_ptr<cartographer_ros_msgs::msg::RobotPose_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const RobotPose_ & other) const
  {
    if (this->robot_pose != other.robot_pose) {
      return false;
    }
    if (this->covariance_score != other.covariance_score) {
      return false;
    }
    if (this->current_trajectory != other.current_trajectory) {
      return false;
    }
    if (this->last_update_pose != other.last_update_pose) {
      return false;
    }
    if (this->last_update_duration != other.last_update_duration) {
      return false;
    }
    return true;
  }
  bool operator!=(const RobotPose_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct RobotPose_

// alias to use template instance with default allocator
using RobotPose =
  cartographer_ros_msgs::msg::RobotPose_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace cartographer_ros_msgs

#endif  // CARTOGRAPHER_ROS_MSGS__MSG__DETAIL__ROBOT_POSE__STRUCT_HPP_
