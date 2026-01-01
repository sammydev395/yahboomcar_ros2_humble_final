// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from cartographer_ros_msgs:msg/RobotPose.idl
// generated code does not contain a copyright notice
#include "cartographer_ros_msgs/msg/detail/robot_pose__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `robot_pose`
// Member `last_update_pose`
#include "geometry_msgs/msg/detail/pose__functions.h"
// Member `current_trajectory`
#include "rosidl_runtime_c/string_functions.h"

bool
cartographer_ros_msgs__msg__RobotPose__init(cartographer_ros_msgs__msg__RobotPose * msg)
{
  if (!msg) {
    return false;
  }
  // robot_pose
  if (!geometry_msgs__msg__Pose__init(&msg->robot_pose)) {
    cartographer_ros_msgs__msg__RobotPose__fini(msg);
    return false;
  }
  // covariance_score
  // current_trajectory
  if (!rosidl_runtime_c__String__init(&msg->current_trajectory)) {
    cartographer_ros_msgs__msg__RobotPose__fini(msg);
    return false;
  }
  // last_update_pose
  if (!geometry_msgs__msg__Pose__init(&msg->last_update_pose)) {
    cartographer_ros_msgs__msg__RobotPose__fini(msg);
    return false;
  }
  // last_update_duration
  return true;
}

void
cartographer_ros_msgs__msg__RobotPose__fini(cartographer_ros_msgs__msg__RobotPose * msg)
{
  if (!msg) {
    return;
  }
  // robot_pose
  geometry_msgs__msg__Pose__fini(&msg->robot_pose);
  // covariance_score
  // current_trajectory
  rosidl_runtime_c__String__fini(&msg->current_trajectory);
  // last_update_pose
  geometry_msgs__msg__Pose__fini(&msg->last_update_pose);
  // last_update_duration
}

bool
cartographer_ros_msgs__msg__RobotPose__are_equal(const cartographer_ros_msgs__msg__RobotPose * lhs, const cartographer_ros_msgs__msg__RobotPose * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // robot_pose
  if (!geometry_msgs__msg__Pose__are_equal(
      &(lhs->robot_pose), &(rhs->robot_pose)))
  {
    return false;
  }
  // covariance_score
  if (lhs->covariance_score != rhs->covariance_score) {
    return false;
  }
  // current_trajectory
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->current_trajectory), &(rhs->current_trajectory)))
  {
    return false;
  }
  // last_update_pose
  if (!geometry_msgs__msg__Pose__are_equal(
      &(lhs->last_update_pose), &(rhs->last_update_pose)))
  {
    return false;
  }
  // last_update_duration
  if (lhs->last_update_duration != rhs->last_update_duration) {
    return false;
  }
  return true;
}

bool
cartographer_ros_msgs__msg__RobotPose__copy(
  const cartographer_ros_msgs__msg__RobotPose * input,
  cartographer_ros_msgs__msg__RobotPose * output)
{
  if (!input || !output) {
    return false;
  }
  // robot_pose
  if (!geometry_msgs__msg__Pose__copy(
      &(input->robot_pose), &(output->robot_pose)))
  {
    return false;
  }
  // covariance_score
  output->covariance_score = input->covariance_score;
  // current_trajectory
  if (!rosidl_runtime_c__String__copy(
      &(input->current_trajectory), &(output->current_trajectory)))
  {
    return false;
  }
  // last_update_pose
  if (!geometry_msgs__msg__Pose__copy(
      &(input->last_update_pose), &(output->last_update_pose)))
  {
    return false;
  }
  // last_update_duration
  output->last_update_duration = input->last_update_duration;
  return true;
}

cartographer_ros_msgs__msg__RobotPose *
cartographer_ros_msgs__msg__RobotPose__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cartographer_ros_msgs__msg__RobotPose * msg = (cartographer_ros_msgs__msg__RobotPose *)allocator.allocate(sizeof(cartographer_ros_msgs__msg__RobotPose), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(cartographer_ros_msgs__msg__RobotPose));
  bool success = cartographer_ros_msgs__msg__RobotPose__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
cartographer_ros_msgs__msg__RobotPose__destroy(cartographer_ros_msgs__msg__RobotPose * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    cartographer_ros_msgs__msg__RobotPose__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
cartographer_ros_msgs__msg__RobotPose__Sequence__init(cartographer_ros_msgs__msg__RobotPose__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cartographer_ros_msgs__msg__RobotPose * data = NULL;

  if (size) {
    data = (cartographer_ros_msgs__msg__RobotPose *)allocator.zero_allocate(size, sizeof(cartographer_ros_msgs__msg__RobotPose), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = cartographer_ros_msgs__msg__RobotPose__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        cartographer_ros_msgs__msg__RobotPose__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
cartographer_ros_msgs__msg__RobotPose__Sequence__fini(cartographer_ros_msgs__msg__RobotPose__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      cartographer_ros_msgs__msg__RobotPose__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

cartographer_ros_msgs__msg__RobotPose__Sequence *
cartographer_ros_msgs__msg__RobotPose__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  cartographer_ros_msgs__msg__RobotPose__Sequence * array = (cartographer_ros_msgs__msg__RobotPose__Sequence *)allocator.allocate(sizeof(cartographer_ros_msgs__msg__RobotPose__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = cartographer_ros_msgs__msg__RobotPose__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
cartographer_ros_msgs__msg__RobotPose__Sequence__destroy(cartographer_ros_msgs__msg__RobotPose__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    cartographer_ros_msgs__msg__RobotPose__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
cartographer_ros_msgs__msg__RobotPose__Sequence__are_equal(const cartographer_ros_msgs__msg__RobotPose__Sequence * lhs, const cartographer_ros_msgs__msg__RobotPose__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!cartographer_ros_msgs__msg__RobotPose__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
cartographer_ros_msgs__msg__RobotPose__Sequence__copy(
  const cartographer_ros_msgs__msg__RobotPose__Sequence * input,
  cartographer_ros_msgs__msg__RobotPose__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(cartographer_ros_msgs__msg__RobotPose);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    cartographer_ros_msgs__msg__RobotPose * data =
      (cartographer_ros_msgs__msg__RobotPose *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!cartographer_ros_msgs__msg__RobotPose__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          cartographer_ros_msgs__msg__RobotPose__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!cartographer_ros_msgs__msg__RobotPose__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
