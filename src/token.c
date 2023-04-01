#include "token.h"
#include "shared.h"
#include "value.h"

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

tokenizer new_tokenizer(const char *filename, const char *stream) {
	return (tokenizer) {
		.stream = stream,
		.filename = filename,
		.line_number = 1,
		.prev = (token) { .kind = TOKEN_KIND_UNDEFINED }
	};
}

#define parse_error(tzr, ...) (\
	fprintf(stderr, "syntax error at %s:%d: ", tzr->filename, tzr->line_number),\
	fprintf(stderr, __VA_ARGS__), \
	fputc('\n', stderr), \
	exit(1))

static char peek(const tokenizer *tzr) {
	return tzr->stream[0];
}

static void advance(tokenizer *tzr) {
	if (peek(tzr) == '\n')
		tzr->line_number++;

	tzr->stream++;
}

static char peek_advance(tokenizer *tzr) {
	char peeked = peek(tzr);
	advance(tzr);
	return peeked;
}

static bool is_alnum_or_underscore(char c) {
	return isalnum(c) || c == '_';
}

static token parse_number(tokenizer *tzr) {
	number num = 0;
	char c;

	while (isdigit(c = peek(tzr))) {
		num = num * 10 + (c - '0');
		advance(tzr);
	}

	if (is_alnum_or_underscore(c))
		parse_error(tzr, "bad character '%c' after integer literal", c);

	return (token) {
		.kind = TOKEN_KIND_LITERAL,
		.val = new_number_value(num)
	};
}

static token parse_identifier(tokenizer *tzr) {
	const char *start = tzr->stream;

	// find the length of the identifier.
	while (is_alnum_or_underscore(peek(tzr)))
		advance(tzr);

	unsigned length = tzr->stream - start;

	// check for predefined identifiers
	if (!strncmp(start, "good", 4)) return (token) { .kind = TOKEN_KIND_LITERAL, .val = VALUE_TRUE };
	if (!strncmp(start, "evil", 4)) return (token) { .kind = TOKEN_KIND_LITERAL, .val = VALUE_FALSE };
	if (!strncmp(start, "chaos_emerald", 13)) return (token) { .kind = TOKEN_KIND_LITERAL, .val = VALUE_NULL };
	if (!strncmp(start, "friend", 6)) return (token) { .kind = TOKEN_KIND_IMPORT };
	if (!strncmp(start, "dr_eggman", 9)) return (token) { .kind = TOKEN_KIND_GLOBAL };
	if (!strncmp(start, "mission", 7)) return (token) { .kind = TOKEN_KIND_FUNCTION };
	if (!strncmp(start, "hedgehog", 8)) return (token) { .kind = TOKEN_KIND_LOCAL };
	if (!strncmp(start, "hmmm", 4)) return (token) { .kind = TOKEN_KIND_IF };
	if (!strncmp(start, "ormaybe", 7)) return (token) { .kind = TOKEN_KIND_ELSE };
	if (!strncmp(start, "loopdeloop", 9)) return (token) { .kind = TOKEN_KIND_WHILE };
	if (!strncmp(start, "eachring", 8)) return (token) { .kind = TOKEN_KIND_FOR };
	if (!strncmp(start, "jump", 4)) return (token) { .kind = TOKEN_KIND_BREAK };
	if (!strncmp(start, "carryon", 7)) return (token) { .kind = TOKEN_KIND_CONTINUE };
	if (!strncmp(start, "nopeseeya", 9)) return (token) { .kind = TOKEN_KIND_RETURN };
	if (!strncmp(start, "start", 5)) return (token) { .kind = TOKEN_KIND_LBRACE };
	if (!strncmp(start, "finish", 5)) return (token) { .kind = TOKEN_KIND_RBRACE };

	// it's a normal identifier, return that.
	return (token) {
		.kind = TOKEN_KIND_IDENTIFIER,
		.identifier = strndup(start, length)
	};
}

static number parse_hex(const tokenizer *tzr, char c) {
	if (isdigit(c))
		return c - '0';

	if ('a' <= c && c <= 'f')
		return (c - 'a') + 10;

	if ('A' <= c && c <= 'F')
		return (c - 'A') + 10;

	parse_error(tzr, "unknown hex digit '%c'", c);
}

static char get_escape_char(tokenizer *tzr) {
	char c = peek_advance(tzr);
	switch (c) {
	case '\'':
	case '\"':
	case '\\':
		return c;

	case 'n': return '\n';
	case 't': return '\t';
	case 'r': return '\r';
	case 'f': return '\f';
	case '0': return '\0';

	case 'x':
		if (tzr->stream[0] == '\0' || tzr->stream[1] == '\0')
			parse_error(tzr, "unterminated '\\x' sequence encountered");

		char upper_nibble = peek_advance(tzr);
		char lower_nibble = peek_advance(tzr);

		return (parse_hex(tzr, upper_nibble) << 4) + parse_hex(tzr, lower_nibble);

	default:
		parse_error(tzr, "unknown escape character '%c'", c);
	}
}

static token parse_string(tokenizer *tzr) {
	char quote = peek_advance(tzr);
	assert(quote == '\'' || quote == '\"');

	unsigned length = 0;
	unsigned capacity = 8;
	char *str = xmalloc(capacity); // no need for `+1` because strings dont have trailing `\0`.

	unsigned starting_line = tzr->line_number;

	char c;
	while ((c = peek_advance(tzr)) != quote) {
		if (c == '\0')
			parse_error(tzr, "unterminated quote encountered starting on line %d", starting_line);

		if (c == '\\')
			c = get_escape_char(tzr);

		if (length == capacity) {
			capacity *= 2;
			str = xrealloc(str, capacity);
		}

		str[length++] = c;
	}

	return (token) {
		.kind = TOKEN_KIND_LITERAL,
		.val = new_string_value(new_string(str, length))
	};
}

static void strip_leading_whitespace_and_comments(tokenizer *tzr) {
	while (true) {
		char c = peek(tzr);

		// EOF encountered, nothin gleft to strip
		if (c == '\0')
			return;

		if (isspace(c)) {
			advance(tzr);
			continue;
		}

		// only c-style line comments are recognized
		if (c == '/' && tzr->stream[1] == '/') {
			while (c != '\0' && c != '\n')
				c = peek_advance(tzr);
			continue;
		}

		return;
	}
}

static token parse_optional_equals(tokenizer *tzr, token_kind if_equal, token_kind if_not_equal) {
	if (peek(tzr) != '=')
		return (token) { .kind = if_not_equal };

	advance(tzr);
	return (token) { .kind = if_equal };
}

token next_token(tokenizer *tzr) {
	strip_leading_whitespace_and_comments(tzr);
	char c = peek(tzr);

	if (c == '\0')
		return (token) { .kind = TOKEN_KIND_UNDEFINED };

	if (isdigit(c))
		return parse_number(tzr);

	if (is_alnum_or_underscore(c))
		return parse_identifier(tzr);

	if (c == '\'' || c == '\"')
		return parse_string(tzr);

	// We don't advance before calling the previous parser kinds.
	advance(tzr);

	switch (c) {
	// Simple tokens
	case '(': return (token) { .kind = TOKEN_KIND_LPAREN };
 	case ')': return (token) { .kind = TOKEN_KIND_RPAREN };
	case '[': return (token) { .kind = TOKEN_KIND_LBRACKET };
	case ']': return (token) { .kind = TOKEN_KIND_RBRACKET };
	case ',': return (token) { .kind = TOKEN_KIND_COMMA };
	case ';': return (token) { .kind = TOKEN_KIND_SEMICOLON };

	// compound tokens
	case '&':
		if ((c = peek_advance(tzr)) != '&')
			parse_error(tzr, "only `&&` is recognized, not `&%c`", c);
		return (token) { .kind = TOKEN_KIND_AND_AND };

	case '|':
		if ((c = peek_advance(tzr)) != '|')
			parse_error(tzr, "only `||` is recognized, not `|%c`", c);
		return (token) { .kind = TOKEN_KIND_OR_OR };

	case '=': return parse_optional_equals(tzr, TOKEN_KIND_EQUAL, TOKEN_KIND_ASSIGN);
	case '!': return parse_optional_equals(tzr, TOKEN_KIND_NOT_EQUAL, TOKEN_KIND_NOT);
	case '<': return parse_optional_equals(tzr, TOKEN_KIND_LESS_THAN_OR_EQUAL, TOKEN_KIND_LESS_THAN);
	case '>': return parse_optional_equals(tzr, TOKEN_KIND_GREATER_THAN_OR_EQUAL, TOKEN_KIND_GREATER_THAN);
	case '+': return parse_optional_equals(tzr, TOKEN_KIND_ADD_ASSIGN, TOKEN_KIND_ADD);
	case '-': return parse_optional_equals(tzr, TOKEN_KIND_SUBTRACT_ASSIGN, TOKEN_KIND_SUBTRACT);
	case '*': return parse_optional_equals(tzr, TOKEN_KIND_MULTIPLY_ASSIGN, TOKEN_KIND_MULTIPLY);
	case '/': return parse_optional_equals(tzr, TOKEN_KIND_DIVIDE_ASSIGN, TOKEN_KIND_DIVIDE);
	case '%': return parse_optional_equals(tzr, TOKEN_KIND_MODULO_ASSIGN, TOKEN_KIND_MODULO);

	default:
		parse_error(tzr, "unknown token start: '%c' (%02x)", c, c);
	}
}

void dump_token(FILE *out, token tkn) {
	switch(tkn.kind) {
	case TOKEN_KIND_UNDEFINED: fputs("UNDEF", out); break;

	case TOKEN_KIND_LITERAL: dump_value(out, tkn.val); break;
	case TOKEN_KIND_IDENTIFIER: fprintf(out, "Identifier(%s)\n", tkn.identifier); break;

	case TOKEN_KIND_IMPORT: fputs("Keyword(friend)", out); break;
	case TOKEN_KIND_GLOBAL: fputs("Keyword(dr_eggman)", out); break;
	case TOKEN_KIND_FUNCTION: fputs("Keyword(mission)", out); break;
	case TOKEN_KIND_LOCAL: fputs("Keyword(hedgehog)", out); break;
	case TOKEN_KIND_IF: fputs("Keyword(hmmm)", out); break;
	case TOKEN_KIND_ELSE: fputs("Keyword(ormaybe)", out); break;
	case TOKEN_KIND_WHILE: fputs("Keyword(loopdeloop)", out); break;
	case TOKEN_KIND_FOR: fputs("Keyword(for)", out); break;
	case TOKEN_KIND_BREAK: fputs("Keyword(jump)", out); break;
	case TOKEN_KIND_CONTINUE: fputs("Keyword(carryon)", out); break;
	case TOKEN_KIND_RETURN: fputs("Keyword(nopeseeya)", out); break;

	case TOKEN_KIND_LPAREN: fputs("Token('(')", out); break;
	case TOKEN_KIND_RPAREN: fputs("Token(')')", out); break;
	case TOKEN_KIND_LBRACKET: fputs("Token('[')", out); break;
	case TOKEN_KIND_RBRACKET: fputs("Token(']')", out); break;
	case TOKEN_KIND_LBRACE: fputs("Keyword(start)", out); break;
	case TOKEN_KIND_RBRACE: fputs("Keyword(finish)", out); break;
	case TOKEN_KIND_COMMA: fputs("Token(',')", out); break;
	case TOKEN_KIND_SEMICOLON: fputs("Token(';')", out); break;
	case TOKEN_KIND_ASSIGN: fputs("Token('=')", out); break;

	case TOKEN_KIND_ADD: fputs("Token('+')", out); break;
	case TOKEN_KIND_SUBTRACT: fputs("Token('-')", out); break;
	case TOKEN_KIND_MULTIPLY: fputs("Token('*')", out); break;
	case TOKEN_KIND_DIVIDE: fputs("Token('/')", out); break;
	case TOKEN_KIND_MODULO: fputs("Token('%')", out); break;

	case TOKEN_KIND_ADD_ASSIGN: fputs("Token('+=')", out); break;
	case TOKEN_KIND_SUBTRACT_ASSIGN: fputs("Token('-=')", out); break;
	case TOKEN_KIND_MULTIPLY_ASSIGN: fputs("Token('*=')", out); break;
	case TOKEN_KIND_DIVIDE_ASSIGN: fputs("Token('/=')", out); break;
	case TOKEN_KIND_MODULO_ASSIGN: fputs("Token('%=')", out); break;

	case TOKEN_KIND_AND_AND: fputs("Token('&&')", out); break;
	case TOKEN_KIND_OR_OR: fputs("Token('||')", out); break;

	case TOKEN_KIND_NOT: fputs("Token('!')", out); break;
	case TOKEN_KIND_EQUAL: fputs("Token('==')", out); break;
	case TOKEN_KIND_NOT_EQUAL: fputs("Token('!=')", out); break;
	case TOKEN_KIND_LESS_THAN: fputs("Token('<')", out); break;
	case TOKEN_KIND_LESS_THAN_OR_EQUAL: fputs("Token('<=')", out); break;
	case TOKEN_KIND_GREATER_THAN: fputs("Token('>')", out); break;
	case TOKEN_KIND_GREATER_THAN_OR_EQUAL: fputs("Token('>=')", out); break;
	}
}
