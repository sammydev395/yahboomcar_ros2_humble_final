// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from yahboomcar_msgs:msg/TargetArray.idl
// generated code does not contain a copyright notice
#include "yahboomcar_msgs/msg/detail/target_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `data`
#include "yahboomcar_msgs/msg/detail/target__functions.h"

bool
yahboomcar_msgs__msg__TargetArray__init(yahboomcar_msgs__msg__TargetArray * msg)
{
  if (!msg) {
    return false;
  }
  // data
  if (!yahboomcar_msgs__msg__Target__Sequence__init(&msg->data, 0)) {
    yahboomcar_msgs__msg__TargetArray__fini(msg);
    return false;
  }
  return true;
}

void
yahboomcar_msgs__msg__TargetArray__fini(yahboomcar_msgs__msg__TargetArray * msg)
{
  if (!msg) {
    return;
  }
  // data
  yahboomcar_msgs__msg__Target__Sequence__fini(&msg->data);
}

bool
yahboomcar_msgs__msg__TargetArray__are_equal(const yahboomcar_msgs__msg__TargetArray * lhs, const yahboomcar_msgs__msg__TargetArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // data
  if (!yahboomcar_msgs__msg__Target__Sequence__are_equal(
      &(lhs->data), &(rhs->data)))
  {
    return false;
  }
  return true;
}

bool
yahboomcar_msgs__msg__TargetArray__copy(
  const yahboomcar_msgs__msg__TargetArray * input,
  yahboomcar_msgs__msg__TargetArray * output)
{
  if (!input || !output) {
    return false;
  }
  // data
  if (!yahboomcar_msgs__msg__Target__Sequence__copy(
      &(input->data), &(output->data)))
  {
    return false;
  }
  return true;
}

yahboomcar_msgs__msg__TargetArray *
yahboomcar_msgs__msg__TargetArray__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__TargetArray * msg = (yahboomcar_msgs__msg__TargetArray *)allocator.allocate(sizeof(yahboomcar_msgs__msg__TargetArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(yahboomcar_msgs__msg__TargetArray));
  bool success = yahboomcar_msgs__msg__TargetArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
yahboomcar_msgs__msg__TargetArray__destroy(yahboomcar_msgs__msg__TargetArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    yahboomcar_msgs__msg__TargetArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
yahboomcar_msgs__msg__TargetArray__Sequence__init(yahboomcar_msgs__msg__TargetArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__TargetArray * data = NULL;

  if (size) {
    data = (yahboomcar_msgs__msg__TargetArray *)allocator.zero_allocate(size, sizeof(yahboomcar_msgs__msg__TargetArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = yahboomcar_msgs__msg__TargetArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        yahboomcar_msgs__msg__TargetArray__fini(&data[i - 1]);
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
yahboomcar_msgs__msg__TargetArray__Sequence__fini(yahboomcar_msgs__msg__TargetArray__Sequence * array)
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
      yahboomcar_msgs__msg__TargetArray__fini(&array->data[i]);
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

yahboomcar_msgs__msg__TargetArray__Sequence *
yahboomcar_msgs__msg__TargetArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__TargetArray__Sequence * array = (yahboomcar_msgs__msg__TargetArray__Sequence *)allocator.allocate(sizeof(yahboomcar_msgs__msg__TargetArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = yahboomcar_msgs__msg__TargetArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
yahboomcar_msgs__msg__TargetArray__Sequence__destroy(yahboomcar_msgs__msg__TargetArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    yahboomcar_msgs__msg__TargetArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
yahboomcar_msgs__msg__TargetArray__Sequence__are_equal(const yahboomcar_msgs__msg__TargetArray__Sequence * lhs, const yahboomcar_msgs__msg__TargetArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!yahboomcar_msgs__msg__TargetArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
yahboomcar_msgs__msg__TargetArray__Sequence__copy(
  const yahboomcar_msgs__msg__TargetArray__Sequence * input,
  yahboomcar_msgs__msg__TargetArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(yahboomcar_msgs__msg__TargetArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    yahboomcar_msgs__msg__TargetArray * data =
      (yahboomcar_msgs__msg__TargetArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!yahboomcar_msgs__msg__TargetArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          yahboomcar_msgs__msg__TargetArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!yahboomcar_msgs__msg__TargetArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
