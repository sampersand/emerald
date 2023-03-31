#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "ast.h"

void dump_value(FILE *out, value val) {
	switch (classify(val)) {
	case VALUE_KIND_BUILTIN_FUNCTION:
		dump_builtin_function(out, as_builtin_function(val));
		break;

	case VALUE_KIND_FUNCTION:
		dump_function(out, as_function(val));
		break;

	case VALUE_KIND_ARRAY:
		dump_array(out, as_array(val));
		break;

	case VALUE_KIND_BOOLEAN:
		fprintf(out, "Boolean(%s)", as_boolean(val) ? "true" : "false");
		break;

	case VALUE_KIND_NULL:
		fputs("Null()", out);
		break;

	case VALUE_KIND_STRING:
		fprintf(out, "String(%.*s)", as_string(val)->length, as_string(val)->ptr);
		break;

	case VALUE_KIND_NUMBER:
		fprintf(out, "Number(%lld)", as_number(val));
		break;
	}
}

void free_value(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		free_string(as_string(val));
		break;

	case VALUE_KIND_ARRAY:
		free_array(as_array(val));
		break;

	case VALUE_KIND_FUNCTION:
		free_function(as_function(val));
		break;

	default:
		// We don't free builtin functions, as they're static for the lifetime of the program.
		break;
	}
}

value clone_value(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		return new_string_value(clone_string(as_string(val)));

	case VALUE_KIND_ARRAY:
		return new_array_value(clone_array(as_array(val)));

	case VALUE_KIND_FUNCTION:
		return new_function_value(clone_function(as_function(val)));

	default:
		return val;
	}
}

const char *value_kind_name(value_kind kind) {
	switch (kind) {
	case VALUE_KIND_BOOLEAN:          return "boolean";
	case VALUE_KIND_NULL:             return "null";
	case VALUE_KIND_STRING:           return "string";
	case VALUE_KIND_ARRAY:            return "array";
	case VALUE_KIND_NUMBER:           return "number";
	case VALUE_KIND_FUNCTION:         return "function";
	case VALUE_KIND_BUILTIN_FUNCTION: return "function"; // Should be indistinguishable from user-defined
	}
}

value call_value(value val, unsigned number_of_locals, const value *argument) {
	switch (classify(val)) {
	case VALUE_KIND_FUNCTION:
		return call_function(as_function(val), number_of_locals, argument);

	case VALUE_KIND_BUILTIN_FUNCTION:
		return call_builtin_function(as_builtin_function(val), number_of_locals, argument);

	default:
		die_with_stacktrace("cannot call a value of kind %s", value_name(val));
	}
}

void index_assign_value(value ary, value idx, value val) {
	if (!is_array(ary))
		die_with_stacktrace("can only index assign into arrays, not %s", value_name(ary));

	if (!is_number(idx))
		die_with_stacktrace("you must index with numbers, not %s", value_name(idx));

	if (!index_assign_array(as_array(ary), as_number(idx), val))
		die_with_stacktrace("cannot assign to negative indices larger than `ary`'s length: %lld", as_number(idx));
}

value index_value(value val, value idx) {
	if (!is_number(idx))
		die_with_stacktrace("you must index with numbers, not %s", value_name(idx));

	number num_idx = as_number(idx);

	switch (classify(val)) {
	case VALUE_KIND_STRING: {
		string *str = index_string(as_string(val), num_idx);

		if (str == NULL)
			die_with_stacktrace("index %lld out of bounds for string of length %u", num_idx, as_string(val)->length);

		return new_string_value(str);
	}

	case VALUE_KIND_ARRAY: {
		value ret = index_array(as_array(val), num_idx);

		if (ret == VALUE_UNDEFINED)
			die_with_stacktrace("index %lld out of bounds for array of length %u", num_idx, as_array(val)->length);

		return ret;
	}

	default:
		die_with_stacktrace("can only index into arrays or strings, not %s", value_name(idx));
	}
}

value negate_value(value val) {
	if (!is_number(val))
		die_with_stacktrace("can only negate numbers, not %s", value_name(val));

	return new_number_value(-as_number(val));
}

value not_value(value val) {
	if (!is_boolean(val))
		die_with_stacktrace("can only not booleans, not %s", value_name(val));

	return new_boolean_value(!as_boolean(val));
}

string *value_to_string(value val) {
	switch (classify(val)) {
	case VALUE_KIND_STRING:
		return clone_string(as_string(val));

	case VALUE_KIND_NUMBER:
		return number_to_string(as_number(val));

	// `new_string` requires ownership of its string, so we `strdup`.
	case VALUE_KIND_BOOLEAN:
		if (val == VALUE_TRUE) {
			return new_string(strdup("good"), 4);
		} else {
			return new_string(strdup("evil"), 4);
		}

	case VALUE_KIND_NULL:
		return new_string(strdup("chaos_emerald"), 13);

	case VALUE_KIND_ARRAY:
		return array_to_string(as_array(val));

	default:
		die_with_stacktrace("no conversion to string defined for %s", value_name(val));
	}
}

value add_values(value lhs, value rhs) {
	// If either side is a string, we convert both to strings then add.
	if (is_string(lhs) || is_string(rhs))
		goto string;

	if (classify(lhs) != classify(rhs)) {
		die_with_stacktrace("can only add like kinds together, or strings to other types, not %s to %s",
			value_name(lhs), value_name(rhs));
	}

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return new_number_value(as_number(lhs) + as_number(rhs));

	case VALUE_KIND_ARRAY:
		return new_array_value(concat_arrays(as_array(lhs), as_array(rhs)));

	case VALUE_KIND_STRING:
	string: {
		string *l = value_to_string(lhs);
		string *r = value_to_string(rhs);

		string *ret = add_strings(l, r);

		free_string(l);
		free_string(r);

		return new_string_value(ret);
	}

	default:
		die_with_stacktrace("can only add numbers, arrays, and strings, not %s", value_name(lhs));
	}
}

value subtract_values(value lhs, value rhs) {
	// Yes its backwards intentionally; English is weird.
	if (!is_number(lhs) || !is_number(rhs)) {
		die_with_stacktrace("can only subtract numbers from numbers, not %s from %s",
			value_name(rhs), value_name(lhs));
	}

	return new_number_value(as_number(lhs) - as_number(rhs));
}

value multiply_values(value lhs, value rhs) {
	if (!is_number(rhs))
		die_with_stacktrace("can only multiply numbers, strings, and arrays by numbers, not %ss", value_name(rhs));

	number amnt = as_number(rhs);

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return new_number_value(as_number(lhs) * amnt);

	case VALUE_KIND_STRING:
		if (amnt < 0)
			die_with_stacktrace("can only multiply strings by nonnegative integers (%lld invalid).", amnt);

		return new_string_value(replicate_string(as_string(lhs), amnt));

	case VALUE_KIND_ARRAY:
		if (amnt < 0)
			die_with_stacktrace("can only multiply arrays by nonnegative integers (%lld invalid).", amnt);

		return new_array_value(replicate_array(as_array(lhs), amnt));

	default:
		die_with_stacktrace("can only multiply numbers, strings, and arrays, not %s.", value_name(lhs));
	}
}

value divide_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die_with_stacktrace("can only divide numbers by numbers, not %s from %s", value_name(lhs), value_name(rhs));

	if (as_number(rhs) == 0)
		die_with_stacktrace("division by zero!");

	return new_number_value(as_number(lhs) / as_number(rhs));
}

value modulo_values(value lhs, value rhs) {
	if (!is_number(lhs) || !is_number(rhs))
		die_with_stacktrace("can only modulo numbers by numbers, not %s from %s", value_name(lhs), value_name(rhs));

	if (as_number(rhs) == 0)
		die_with_stacktrace("modulo by zero!");

	return new_number_value(as_number(lhs) % as_number(rhs));
}

int compare_values(value lhs, value rhs) {
	if (classify(lhs) != classify(rhs))
		die_with_stacktrace("can only compare like kinds together, not %s to %s", value_name(lhs), value_name(rhs));

	switch (classify(lhs)) {
	case VALUE_KIND_NUMBER:
		return compare_numbers(as_number(lhs), as_number(rhs));

	case VALUE_KIND_ARRAY:
		return compare_arrays(as_array(lhs), as_array(rhs));

	case VALUE_KIND_STRING:
		return compare_strings(as_string(lhs), as_string(rhs));

	default:
		die_with_stacktrace("can only compare numbers, arrays, and strings, not %s", value_name(lhs));
	}
}

bool equate_values(value lhs, value rhs) {
	// if the two values have identical representation, then they're the same.
	if (lhs == rhs)
		return true;

	// If they're not the same type, they're not equal.
	if (classify(lhs) != classify(rhs))
		return false;

	switch (classify(lhs)) {
	case VALUE_KIND_BOOLEAN:
	case VALUE_KIND_NULL:
	case VALUE_KIND_NUMBER:
	case VALUE_KIND_FUNCTION:
	case VALUE_KIND_BUILTIN_FUNCTION:
		return false; // If `lhs` isn't identical to `rhs`, then they're not equivalent.

	case VALUE_KIND_STRING:
		return equate_strings(as_string(lhs), as_string(rhs));

	case VALUE_KIND_ARRAY:
		return equate_arrays(as_array(lhs), as_array(rhs));
	}
}

string *inspect_value(value val) {
	return is_string(val) ? inspect_string(as_string(val)) : value_to_string(val);
}
