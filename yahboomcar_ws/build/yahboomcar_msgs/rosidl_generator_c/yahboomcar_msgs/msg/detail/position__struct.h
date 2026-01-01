// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from yahboomcar_msgs:msg/Position.idl
// generated code does not contain a copyright notice

#ifndef YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_H_
#define YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/Position in the package yahboomcar_msgs.
typedef struct yahboomcar_msgs__msg__Position
{
  float anglex;
  float angley;
  float distance;
} yahboomcar_msgs__msg__Position;

// Struct for a sequence of yahboomcar_msgs__msg__Position.
typedef struct yahboomcar_msgs__msg__Position__Sequence
{
  yahboomcar_msgs__msg__Position * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} yahboomcar_msgs__msg__Position__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // YAHBOOMCAR_MSGS__MSG__DETAIL__POSITION__STRUCT_H_
