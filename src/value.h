#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "valuedefn.h"
#include "shared.h"
#include "string.h"
#include "array.h"
#include "number.h"
#include "function.h"
#include "builtin_function.h"
#include "ast.h"

#define VALUE_FALSE 0
#define VALUE_NULL 1
#define VALUE_TRUE 2
#define VALUE_UNDEFINED 3

/*
As Friar is a dynamically typed langauge, all possible types are encoded within
a single `value` type. To differentiate between types, we use encode type data
via bit masking: The bottom three bits are used to distinguish the different
types of values. This takes advantage of the fact that all the pointer types
(i.e. `string`, `function`, `array`, and `builtin_function`) are all 8 aligned,
which means the bottom three bits are used for storing information.

As such, `number` is really a 61 bit integer, as three bits are used for tagging.

The scheme is laid out as follows:
000...000 = VALUE_FALSE
000...001 = VALUE_NULL
000...010 = VALUE_TRUE
000...011 = VALUE_UNDEFINED (indicates "undefined")
XXX...000 = string
XXX...001 = user-defined function
XXX...010 = ary
XXX...011 = builtin function
XXX...100 = number
*/
enum {
	VALUE_TAG_STRING           = 0,
	VALUE_TAG_FUNCTION         = 1,
	VALUE_TAG_ARRAY            = 2,
	VALUE_TAG_BUILTIN_FUNCTION = 3,
	VALUE_TAG_NUMBER           = 4,
	VALUE_TAG_MASK             = 7,
};

/*
 * An enum used to indicate what type a `value` is.
 *
 * Note that while this is separate from the `VALUE_TAG_`s, the values _with_ tags use
 * their tags as it provides seamless conversion.
 */
typedef enum {
	VALUE_KIND_STRING           = VALUE_TAG_STRING,
	VALUE_KIND_FUNCTION         = VALUE_TAG_FUNCTION,
	VALUE_KIND_BUILTIN_FUNCTION = VALUE_TAG_BUILTIN_FUNCTION,
	VALUE_KIND_ARRAY            = VALUE_TAG_ARRAY,
	VALUE_KIND_NUMBER           = VALUE_TAG_NUMBER,
	VALUE_KIND_BOOLEAN          = VALUE_TAG_MASK + 1, // guaranteed to not be a tag
	VALUE_KIND_NULL             = VALUE_TAG_MASK + 2, // also guaranteed not to eb a tag.
} value_kind;

// Returns a string representation of `kind`.
const char *value_kind_name(value_kind kind);

// Returns the kind of value `val` is.
static inline value_kind classify(value val) {
	assert(val != VALUE_UNDEFINED);

	if (val == VALUE_NULL)
		return VALUE_KIND_NULL;

	if (val == VALUE_TRUE || val == VALUE_FALSE)
		return VALUE_KIND_BOOLEAN;

	return val & VALUE_TAG_MASK;
}

// Returns a string representation of `val`'s kind.
static inline const char *value_name(value val) {
	return value_kind_name(classify(val));
}

// Creates a new `value` out of a `bool`.
static inline value new_boolean_value(bool boolean) {
	return boolean ? VALUE_TRUE : VALUE_FALSE;
}

// Creates a new `value` out of an `array`.
static inline value new_array_value(array *ary) {
	assert(((value) ary & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) ary | VALUE_TAG_ARRAY;
}

// Creates a new `value` out of a `number`.
static inline value new_number_value(number num) {
	return ((value) num << 3) | VALUE_TAG_NUMBER;
}

// Creates a new `value` out of a `string`.
static inline value new_string_value(string *str) {
	assert(((value) str & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) str | VALUE_TAG_STRING;
}

// Creates a new `value` out of a `function`.
static inline value new_function_value(function *func) {
	assert(((value) func & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) func | VALUE_TAG_FUNCTION;
}

// Creates a new `value` out of a `builtin_function`.
static inline value new_builtin_function_value(builtin_function *builtin_func) {
	assert(((value) builtin_func & VALUE_TAG_MASK) == 0); // Sanity check for alignment.
	return (value) builtin_func | VALUE_TAG_BUILTIN_FUNCTION;
}

// Note that there's no `is_null` as you can just do `== NULL`

// Checks if `val` is a `bool`.
static inline bool is_boolean(value val) {
	return val == VALUE_TRUE || val == VALUE_FALSE;
}

// Checks if `val` is an `array`.
static inline bool is_array(value val) {
	return classify(val) == VALUE_KIND_ARRAY;
}

// Checks if `val` is a `number`.
static inline bool is_number(value val) {
	return classify(val) == VALUE_KIND_NUMBER;
}

// Checks if `val` is a `string`.
static inline bool is_string(value val) {
	return classify(val) == VALUE_KIND_STRING;
}

// Checks if `val` is a `function`.
static inline bool is_function(value val) {
	return classify(val) == VALUE_KIND_FUNCTION;
}

// Checks if `val` is a `builtin_function`.
static inline bool is_builtin_function(value val) {
	return classify(val) == VALUE_KIND_BUILTIN_FUNCTION;
}

// Casts `val` to a `bool` without verifying its type.
static inline bool as_boolean(value val) {
	assert(is_boolean(val));
	return val == VALUE_TRUE;
}

// Casts `val` to a `number` without verifying its type.
static inline number as_number(value val) {
	assert(is_number(val));
	return (number) val >> 3;
}

// Casts `val` to an `array` without verifying its type.
static inline array *as_array(value val) {
	assert(is_array(val));
	return (array *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `string` without verifying its type.
static inline string *as_string(value val) {
	assert(is_string(val));
	return (string *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `function` without verifying its type.
static inline function *as_function(value val) {
	assert(is_function(val));
	return (function *) (val & ~VALUE_TAG_MASK);
}

// Casts `val` to a `builtin_function` without verifying its type.
static inline builtin_function *as_builtin_function(value val) {
	assert(is_builtin_function(val));
	return (builtin_function *) (val & ~VALUE_TAG_MASK);
}

// Dumps a debug representation of `val` to `out`.
void dump_value(FILE *out, value val);

// Frees `val`; as `val`s have reference-counting semantics, use `free_value`
// when you're done with a value.
void free_value(value val);

// Clones `val`; as `val`s have reference-counting semantics, use `clone_value`
// when you need to duplicate ownership.
value clone_value(value val);

// Converts `val` to a string.
string *value_to_string(value val);

// Gets a debugging representation of `val`.
string *inspect_value(value val);

// Calls `val` with the given arguments.
value call_value(value val, unsigned number_of_arguments, const value *arguments);

// Numerically negates `val`.
value negate_value(value val);

// Logically negates `val`.
value not_value(value val);

// Adds `lhs` to `rhs`.
value add_values(value lhs, value rhs);

// Subtracts `rhs` from `lhs`.
value subtract_values(value lhs, value rhs);

// Multiplies `lhs` with `rhs`.
value multiply_values(value lhs, value rhs);

// Divides `lhs` by `rhs`.
value divide_values(value lhs, value rhs);

// Modulos `lhs` by `rhs`.
value modulo_values(value lhs, value rhs);

// Returns true if `lhs` equals `rhs`.
bool equate_values(value lhs, value rhs);

// Returns a negative, zero, or positive number depending on if `lhs` is
// less than, equal to, or greater than `rhs`.
int compare_values(value lhs, value rhs);

// Gets the element at `idx` within `source`.
value index_value(value source, value idx);

// Assigns the element at `idx` to `val` within `source`.
void index_assign_value(value source, value idx, value val);
