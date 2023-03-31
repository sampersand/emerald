#pragma once

// This file is needed because many other files (such as `array.h`) rely upon `value`
// being defined. However, `value.h` relies upon `array` and themselves friends being
// defined for functions like creation and casting. So, to break the circular dependency, this file
// exists solely to define `value`.

// The value type. Note that we have reference counting semantics.
typedef unsigned long long value;

// Since we do bit packing, we need this minimum alignment.
#define VALUE_ALIGNMENT _Alignas(8)
