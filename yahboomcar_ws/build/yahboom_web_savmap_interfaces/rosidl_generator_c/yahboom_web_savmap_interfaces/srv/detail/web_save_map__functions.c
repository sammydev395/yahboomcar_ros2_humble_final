// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from yahboom_web_savmap_interfaces:srv/WebSaveMap.idl
// generated code does not contain a copyright notice
#include "yahboom_web_savmap_interfaces/srv/detail/web_save_map__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"

// Include directives for member types
// Member `mapname`
#include "rosidl_runtime_c/string_functions.h"

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg)
{
  if (!msg) {
    return false;
  }
  // mapname
  if (!rosidl_runtime_c__String__init(&msg->mapname)) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(msg);
    return false;
  }
  return true;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg)
{
  if (!msg) {
    return;
  }
  // mapname
  rosidl_runtime_c__String__fini(&msg->mapname);
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // mapname
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->mapname), &(rhs->mapname)))
  {
    return false;
  }
  return true;
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * output)
{
  if (!input || !output) {
    return false;
  }
  // mapname
  if (!rosidl_runtime_c__String__copy(
      &(input->mapname), &(output->mapname)))
  {
    return false;
  }
  return true;
}

yahboom_web_savmap_interfaces__srv__WebSaveMap_Request *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Request *)allocator.allocate(sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request));
  bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * data = NULL;

  if (size) {
    data = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Request *)allocator.zero_allocate(size, sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(&data[i - 1]);
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
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array)
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
      yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(&array->data[i]);
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

yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence *)allocator.allocate(sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Request);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Request * data =
      (yahboom_web_savmap_interfaces__srv__WebSaveMap_Request *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Request__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}


// Include directives for member types
// Member `response`
// already included above
// #include "rosidl_runtime_c/string_functions.h"

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg)
{
  if (!msg) {
    return false;
  }
  // response
  if (!rosidl_runtime_c__String__init(&msg->response)) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(msg);
    return false;
  }
  return true;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg)
{
  if (!msg) {
    return;
  }
  // response
  rosidl_runtime_c__String__fini(&msg->response);
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // response
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->response), &(rhs->response)))
  {
    return false;
  }
  return true;
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * output)
{
  if (!input || !output) {
    return false;
  }
  // response
  if (!rosidl_runtime_c__String__copy(
      &(input->response), &(output->response)))
  {
    return false;
  }
  return true;
}

yahboom_web_savmap_interfaces__srv__WebSaveMap_Response *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Response *)allocator.allocate(sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response));
  bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__init(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * data = NULL;

  if (size) {
    data = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Response *)allocator.zero_allocate(size, sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(&data[i - 1]);
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
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__fini(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array)
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
      yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(&array->data[i]);
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

yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence *
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array = (yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence *)allocator.allocate(sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__destroy(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__are_equal(const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * lhs, const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence__copy(
  const yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * input,
  yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(yahboom_web_savmap_interfaces__srv__WebSaveMap_Response);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    yahboom_web_savmap_interfaces__srv__WebSaveMap_Response * data =
      (yahboom_web_savmap_interfaces__srv__WebSaveMap_Response *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!yahboom_web_savmap_interfaces__srv__WebSaveMap_Response__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
