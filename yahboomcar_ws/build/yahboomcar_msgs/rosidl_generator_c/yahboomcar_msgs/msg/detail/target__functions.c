// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from yahboomcar_msgs:msg/Target.idl
// generated code does not contain a copyright notice
#include "yahboomcar_msgs/msg/detail/target__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `frame_id`
#include "rosidl_runtime_c/string_functions.h"
// Member `stamp`
#include "builtin_interfaces/msg/detail/time__functions.h"

bool
yahboomcar_msgs__msg__Target__init(yahboomcar_msgs__msg__Target * msg)
{
  if (!msg) {
    return false;
  }
  // frame_id
  if (!rosidl_runtime_c__String__init(&msg->frame_id)) {
    yahboomcar_msgs__msg__Target__fini(msg);
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__init(&msg->stamp)) {
    yahboomcar_msgs__msg__Target__fini(msg);
    return false;
  }
  // scores
  // ptx
  // pty
  // distw
  // disth
  // centerx
  // centery
  return true;
}

void
yahboomcar_msgs__msg__Target__fini(yahboomcar_msgs__msg__Target * msg)
{
  if (!msg) {
    return;
  }
  // frame_id
  rosidl_runtime_c__String__fini(&msg->frame_id);
  // stamp
  builtin_interfaces__msg__Time__fini(&msg->stamp);
  // scores
  // ptx
  // pty
  // distw
  // disth
  // centerx
  // centery
}

bool
yahboomcar_msgs__msg__Target__are_equal(const yahboomcar_msgs__msg__Target * lhs, const yahboomcar_msgs__msg__Target * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // frame_id
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->frame_id), &(rhs->frame_id)))
  {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__are_equal(
      &(lhs->stamp), &(rhs->stamp)))
  {
    return false;
  }
  // scores
  if (lhs->scores != rhs->scores) {
    return false;
  }
  // ptx
  if (lhs->ptx != rhs->ptx) {
    return false;
  }
  // pty
  if (lhs->pty != rhs->pty) {
    return false;
  }
  // distw
  if (lhs->distw != rhs->distw) {
    return false;
  }
  // disth
  if (lhs->disth != rhs->disth) {
    return false;
  }
  // centerx
  if (lhs->centerx != rhs->centerx) {
    return false;
  }
  // centery
  if (lhs->centery != rhs->centery) {
    return false;
  }
  return true;
}

bool
yahboomcar_msgs__msg__Target__copy(
  const yahboomcar_msgs__msg__Target * input,
  yahboomcar_msgs__msg__Target * output)
{
  if (!input || !output) {
    return false;
  }
  // frame_id
  if (!rosidl_runtime_c__String__copy(
      &(input->frame_id), &(output->frame_id)))
  {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__copy(
      &(input->stamp), &(output->stamp)))
  {
    return false;
  }
  // scores
  output->scores = input->scores;
  // ptx
  output->ptx = input->ptx;
  // pty
  output->pty = input->pty;
  // distw
  output->distw = input->distw;
  // disth
  output->disth = input->disth;
  // centerx
  output->centerx = input->centerx;
  // centery
  output->centery = input->centery;
  return true;
}

yahboomcar_msgs__msg__Target *
yahboomcar_msgs__msg__Target__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__Target * msg = (yahboomcar_msgs__msg__Target *)allocator.allocate(sizeof(yahboomcar_msgs__msg__Target), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(yahboomcar_msgs__msg__Target));
  bool success = yahboomcar_msgs__msg__Target__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
yahboomcar_msgs__msg__Target__destroy(yahboomcar_msgs__msg__Target * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    yahboomcar_msgs__msg__Target__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
yahboomcar_msgs__msg__Target__Sequence__init(yahboomcar_msgs__msg__Target__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__Target * data = NULL;

  if (size) {
    data = (yahboomcar_msgs__msg__Target *)allocator.zero_allocate(size, sizeof(yahboomcar_msgs__msg__Target), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = yahboomcar_msgs__msg__Target__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        yahboomcar_msgs__msg__Target__fini(&data[i - 1]);
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
yahboomcar_msgs__msg__Target__Sequence__fini(yahboomcar_msgs__msg__Target__Sequence * array)
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
      yahboomcar_msgs__msg__Target__fini(&array->data[i]);
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

yahboomcar_msgs__msg__Target__Sequence *
yahboomcar_msgs__msg__Target__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboomcar_msgs__msg__Target__Sequence * array = (yahboomcar_msgs__msg__Target__Sequence *)allocator.allocate(sizeof(yahboomcar_msgs__msg__Target__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = yahboomcar_msgs__msg__Target__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
yahboomcar_msgs__msg__Target__Sequence__destroy(yahboomcar_msgs__msg__Target__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    yahboomcar_msgs__msg__Target__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
yahboomcar_msgs__msg__Target__Sequence__are_equal(const yahboomcar_msgs__msg__Target__Sequence * lhs, const yahboomcar_msgs__msg__Target__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!yahboomcar_msgs__msg__Target__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
yahboomcar_msgs__msg__Target__Sequence__copy(
  const yahboomcar_msgs__msg__Target__Sequence * input,
  yahboomcar_msgs__msg__Target__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(yahboomcar_msgs__msg__Target);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    yahboomcar_msgs__msg__Target * data =
      (yahboomcar_msgs__msg__Target *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!yahboomcar_msgs__msg__Target__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          yahboomcar_msgs__msg__Target__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!yahboomcar_msgs__msg__Target__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
