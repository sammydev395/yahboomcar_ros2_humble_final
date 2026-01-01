// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from yahboomcar_msgs:msg/TargetArray.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_H_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'data'
#include "yahboomcar_msgs/msg/detail/target__struct.h"

/// Struct defined in msg/TargetArray in the package yahboomcar_msgs.
typedef struct yahboomcar_msgs__msg__TargetArray
{
  yahboomcar_msgs__msg__Target__Sequence data;
} yahboomcar_msgs__msg__TargetArray;

// Struct for a sequence of yahboomcar_msgs__msg__TargetArray.
typedef struct yahboomcar_msgs__msg__TargetArray__Sequence
{
  yahboomcar_msgs__msg__TargetArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} yahboomcar_msgs__msg__TargetArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__TARGET_ARRAY__STRUCT_H_
