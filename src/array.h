#pragma once

#include <assert.h>
#include <stdbool.h>
#include "shared.h"
#include "valuedefn.h"
#include "string.h"
#include <stdio.h>

/** The array type within Friar.
 */
typedef struct {
	VALUE_ALIGNMENT value *elements;
	unsigned refcount, length, capacity;
} array;

/** Creates a new array with the given elements, length, and capacity.
 * 
 * This takes ownership of `elements`. Note that `capacity` must be at least `length`.
 */
array *new_array(value *elements, unsigned length, unsigned capacity);

/** Allocates an array which can old at least `capacity` elements.
 */
static inline array *allocate_array(unsigned capacity) {
	return new_array(xmalloc(sizeof(value) * capacity), 0, capacity);
}


/** Deallocates `ary`'s memory.
 * 
 * This should only be called when `ary->refcount` is 0. Prefer `free_array` unless you're certain
 * that `ary` has no more references left to it.
 */
void deallocate_array(array *ary);

/** Indicates that we no longer have ownership of `ary`.
 * 
 * This will decrement the refcount, deallocating the array if no other references to it exist.
 */
static inline void free_array(array *ary) {
	assert(ary->refcount != 0); // This means it should have been freed already.

	ary->refcount--;
	if (ary->refcount == 0)
		deallocate_array(ary);
}

/** Increments `ary`'s refcount, returning `ary`.
 * 
 * This is essentially a convenience function.
 */
static inline array *clone_array(array *ary) {
	assert(ary->refcount != 0); // This means it should have been freed already.
	ary->refcount++;
	return ary;
}

/** Pushes `val` onto the end of `ary`.
 * 
 * Ownership of `val` is passed to `ary`.
 */
void push_array(array *ary, value val);

/** Removes the last value from `ary`.
 * 
 * This returns `VALUE_UNDEFINED` if `ary` is empty. 
 * 
 * Ownership of the returned value is given to the caller.
 */
value pop_array(array *ary);

/** Gets the value at `index` within `ary`.
 * 
 * This returning `VALUE_UNDEFINED` if `index` is out of bounds.
 * 
 * If `index` is negative, instead index from the end.
 * 
 * The returned value is owned by the caller.
 */
value index_array(const array *ary, int index);

/** Sets the value at `index` to `val`.
 * 
 * This gives ownership of `val` to `ary`.
 * 
 * If `index` is negative, instead index from the end. If `index` is out of bounds, false is
 * returned.
 * 
 * If `index` is larger than `ary`'s length, missing elements are set to `null`.
 */
bool index_assign_array(array *ary, int index, value val);

/** Deletes the value at `index` within `ary` and returns it.
 * 
 * This returning `VALUE_UNDEFINED` if `index` is out of bounds.
 * 
 * If `index` is negative, instead index from the end.
 * 
 * The returned value is owned by the caller.
 */
value delete_at_array(array *ary, int index);

/** Insets the value `val` at `index` within `ary`. 
 *
 * If `index` is negative, instead index from the end. If `index` is out of bounds, false is
 * returned.
 * 
 * If `index` is larger than `ary`'s length, missing elements are set to `null`.
 * 
 * Ownership of `val` is given to `ary`.
 */
bool insert_at_array(array *ary, int index, value val);

/** Returns a new array with `lhs` and `rhs` combined.
 */
array *concat_arrays(const array *lhs, const array *rhs);

/** Compares `lhs` to `rhs`, returning a negative, zero, or positive number if `lhs` is less than,
 * equal to, or greater than `rhs`.
 * 
 * More specifically, this will go value-by-value (stopping at the end of the shorter array), and
 * returning if any value from `lhs` isn't equal its respective value `rhs`. If all elements are
 * equal, then the lengths of `lhs` and `rhs` are compared and returned.
 */
int compare_arrays(const array *lhs, const array *rhs);
bool equate_arrays(const array *lhs, const array *rhs);
array *replicate_array(array *ary, unsigned amnt);

string *array_to_string(const array *ary);
void dump_array(FILE *out, const array *ary);
