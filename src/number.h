#pragma once
#include "string.h"

typedef long long number;

// `compare_numbers` returns a negative, zero, or positive number depending on whether `lhs` is less
// than, equal to, or greater than `rhs`. So, we can just subtract the two.
static inline int compare_numbers(number lhs, number rhs) {
	return lhs - rhs;
}

string *number_to_string(number num);
number string_to_number(const string *str);
