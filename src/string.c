#include "string.h"
#include "shared.h"
#include <assert.h>
#include <ctype.h>

string *new_string(char *ptr, unsigned length) {
	string *str = xmalloc(sizeof(string));

	str->refcount = 1;
	str->length = length;
	str->ptr = ptr;

	return str;
}

void deallocate_string(string *str) {
	assert(str->refcount == 0);

	free(str->ptr);
	free(str);
}

string *index_string(const string *str, int idx) {
	if (idx < 0) {
		idx += str->length;

		if (idx < 0)
			return NULL;
	}

	if (str->length <= (unsigned) idx)
		return NULL;

	string *ret = allocate_string(1);
	ret->ptr[0] = str->ptr[idx];
	ret->length = 1;

	return ret;
}

string *add_strings(string *lhs, string *rhs) {
	if (lhs->length == 0)
		return clone_string(rhs);

	if (rhs->length == 0)
		return clone_string(lhs);

	string *str = allocate_string(lhs->length + rhs->length);
	memcpy(str->ptr, lhs->ptr, lhs->length);
	memcpy(str->ptr + lhs->length, rhs->ptr, rhs->length);
	str->length = lhs->length + rhs->length;

	return str;
}

int compare_strings(const string *lhs, const string *rhs) {
	unsigned min_len = lhs->length < rhs->length ? lhs->length : rhs->length;

	// Compare the bytes of the strings to begin with. If they're not equal, then return that.
	int cmp = memcmp(lhs->ptr, rhs->ptr, min_len);
	if (cmp != 0)
		return cmp;

	// Otherwise, the shorter string is smaller.
	// If they have the same length and same `cmp`, they're equal
	return lhs->length - rhs->length;
}

bool equate_strings(const string *lhs, const string *rhs) {
	if (lhs->length != rhs->length)
		return false;

	return !memcmp(lhs->ptr, rhs->ptr, lhs->length);
}

string *replicate_string(string *str, unsigned amnt) {
	if (amnt == 1)
		return clone_string(str);

	string *ret = allocate_string(str->length * amnt);

	for (unsigned i = 0; i < amnt; i++)
		memcpy(ret->ptr + i*str->length, str->ptr, str->length);

	ret->length = str->length * amnt;

	return ret;
}

char *new_cstr_from_string(const string *str) {
	for (unsigned i = 0; i < str->length; i++) {
		if (str->ptr[i] == '\0')
			return NULL;
	}

	char *cstr = xmalloc(str->length + 1);
	memcpy(cstr, str->ptr, str->length);
	cstr[str->length] = '\0';
	return cstr;
}

static void push_string(string *str, unsigned *capacity, char chr) {
	if (*capacity <= str->length) {
		*capacity *= 2;
		str->ptr = xrealloc(str->ptr, *capacity);
	}

	str->ptr[str->length] = chr;
	str->length++;
}

string *inspect_string(const string *str) {
	string *inspected = allocate_string(str->length + 2); // the 2 is for the quotes
	unsigned capacity = str->length + 2;

	push_string(inspected, &capacity, '"');

	for (unsigned i = 0; i < str->length; ++i) {
		char chr = str->ptr[i];

		switch (chr) {
		case '\n': chr = 'n'; goto slash;
		case '\t': chr = 't'; goto slash;
		case '\r': chr = 'r'; goto slash;
		case '\0': chr = '0'; goto slash;
		case '\f': chr = 'f'; goto slash;
		case '\\':
		case '\"':
		case '\'':
		slash:
			push_string(inspected, &capacity, '\\');
			break;

		default:
			if (isprint(chr))
				break;

			push_string(inspected, &capacity, '\\');
			push_string(inspected, &capacity, 'x');
			push_string(inspected, &capacity, '0' + (chr >> 4));
			push_string(inspected, &capacity, '0' + (chr & 0xf));
			continue;
		}

		push_string(inspected, &capacity, chr);
	}

	push_string(inspected, &capacity, '"');

	return inspected;
}
