#pragma once
#include <stdio.h>
#include "valuedefn.h"

typedef enum {
	// Indicates that the token isn't actually a token.
	// This is used for things like marking the end of a stream.
	TOKEN_KIND_UNDEFINED,

	// These two make use of the `union` within the `token`.
	TOKEN_KIND_LITERAL, TOKEN_KIND_IDENTIFIER,

	// Keywords
	TOKEN_KIND_IMPORT,
	TOKEN_KIND_GLOBAL, TOKEN_KIND_FUNCTION, TOKEN_KIND_LOCAL,
	TOKEN_KIND_IF, TOKEN_KIND_ELSE,
	TOKEN_KIND_WHILE, TOKEN_KIND_FOR, TOKEN_KIND_BREAK, TOKEN_KIND_CONTINUE,
	TOKEN_KIND_RETURN,

	// Punctuation
	TOKEN_KIND_LPAREN, TOKEN_KIND_RPAREN,
	TOKEN_KIND_LBRACKET, TOKEN_KIND_RBRACKET,
	TOKEN_KIND_LBRACE, TOKEN_KIND_RBRACE,
	TOKEN_KIND_COMMA, TOKEN_KIND_SEMICOLON,
	TOKEN_KIND_ASSIGN,

	// Math operators and their assignment overloads.
	TOKEN_KIND_ADD, TOKEN_KIND_SUBTRACT,
	TOKEN_KIND_MULTIPLY, TOKEN_KIND_DIVIDE, TOKEN_KIND_MODULO,

	// Math assignment operators
	TOKEN_KIND_ADD_ASSIGN, TOKEN_KIND_SUBTRACT_ASSIGN,
	TOKEN_KIND_MULTIPLY_ASSIGN, TOKEN_KIND_DIVIDE_ASSIGN, TOKEN_KIND_MODULO_ASSIGN,

	// Short circuit operators
	TOKEN_KIND_AND_AND, TOKEN_KIND_OR_OR,

	// Logical operators
	TOKEN_KIND_NOT,
	TOKEN_KIND_EQUAL, TOKEN_KIND_NOT_EQUAL,
	TOKEN_KIND_LESS_THAN, TOKEN_KIND_LESS_THAN_OR_EQUAL,
	TOKEN_KIND_GREATER_THAN, TOKEN_KIND_GREATER_THAN_OR_EQUAL,
} token_kind;

typedef struct {
	token_kind kind;
	union {
		value val;
		char *identifier;
	};
} token;

typedef struct {
	const char *stream, *filename;
	unsigned line_number;
	token prev;
} tokenizer;

tokenizer new_tokenizer(const char *filename, const char *stream);
token next_token(tokenizer *tzr);
void dump_token(FILE *out, token tkn);
