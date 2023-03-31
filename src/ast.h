#pragma once

#include "valuedefn.h"
#include "token.h"

typedef struct ast_expression ast_expression;
typedef struct ast_primary ast_primary;
typedef struct ast_block ast_block;
typedef struct ast_statement ast_statement;
typedef struct ast_declaration ast_declaration;

typedef enum {
	UNARY_OP_NEGATE,
	UNARY_OP_NOT
} unary_operator;

struct ast_primary {
	enum {
		AST_PRIMARY_PAREN,
		AST_PRIMARY_INDEX,
		AST_PRIMARY_FUNCTION_CALL,
		AST_PRIMARY_UNARY_OPERATOR,
		AST_PRIMARY_ARRAY_LITERAL,
		AST_PRIMARY_VARIABLE,
		AST_PRIMARY_LITERAL
	} kind;

	union {
		struct {
			ast_expression *expression;
		} paren;

		struct {
			ast_primary *source;
			ast_expression *index;
		} index;

		struct {
			ast_primary *function;

			unsigned number_of_arguments;
			ast_expression **arguments;
		} function_call;

		struct {
			unary_operator operator;
			ast_primary *primary;
		} unary_operator;

		struct {
			unsigned length;
			ast_expression **elements;
		} array_literal;

		struct {
			char *name;
		} variable;

		struct {
			value val;
		} literal;
	};
};

typedef enum {
	SHORT_CIRCUIT_AND_AND,
	SHORT_CIRCUIT_OR_OR
} short_circuit_operator;

typedef enum {
	BINARY_OP_UNDEF,
	BINARY_OP_ADD,
	BINARY_OP_SUBTRACT,
	BINARY_OP_MULTIPLY,
	BINARY_OP_DIVIDE,
	BINARY_OP_MODULO,
	BINARY_OP_EQUAL,
	BINARY_OP_NOT_EQUAL,
	BINARY_OP_LESS_THAN,
	BINARY_OP_LESS_THAN_OR_EQUAL,
	BINARY_OP_GREATER_THAN,
	BINARY_OP_GREATER_THAN_OR_EQUAL,
} binary_operator;

struct ast_expression {
	enum {
		AST_EXPRESSION_ASSIGN,
		AST_EXPRESSION_INDEX_ASSIGN,
		AST_EXPRESSION_SHORT_CIRCUIT_OPERATOR,
		AST_EXPRESSION_BINARY_OPERATOR,
		AST_EXPRESSION_PRIMARY
	} kind;

	union {
		struct {
			binary_operator operator; // Set to `BINARY_OP_UNDEF` when normal assignment.
			char *name;
			ast_expression *value;
		} assign;

		struct {
			binary_operator operator; // Set to `BINARY_OP_UNDEF` when normal assignment.
			ast_primary *source;
			ast_expression *index, *value;
		} index_assign;

		struct {
			short_circuit_operator operator;
			ast_primary *lhs;
			ast_expression *rhs;
		} short_circuit_operator;

		struct {
			binary_operator operator;
			ast_primary *lhs;
			ast_expression *rhs;
		} binary_operator;

		ast_primary *primary;
	};
};

struct ast_statement {
	enum {
		AST_STATEMENT_LOCAL,
		AST_STATEMENT_RETURN,
		AST_STATEMENT_IF,
		AST_STATEMENT_WHILE,
		AST_STATEMENT_BREAK,
		AST_STATEMENT_CONTINUE,
		AST_STATEMENT_EXPRESSION
	} kind;

	union {
		struct {
			char *name;
			ast_expression *initializer; // is `NULL` if no default is given.
		} local;

		struct {
			ast_expression *expression; // is `NULL` if there's no explicit value given.
		} return_;

		struct {
			ast_expression *condition;
			ast_block *if_true, *if_false; // `if_false` is `NULL` if there wasn't an `else`.
		} if_;

		struct {
			ast_expression *condition;
			ast_block *body;
		} while_;

		// nothing for `break` or `continue`, as those dont have associated data.
		ast_expression *expression;
	};
};

struct ast_block {
	unsigned number_of_statements;
	ast_statement **statements;
};

struct ast_declaration {
	enum {
		AST_DECLARATION_IMPORT,
		AST_DECLARATION_GLOBAL,
		AST_DECLARATION_FUNCTION
	} kind;

	struct {
		const char *filename;
		unsigned line_number;
	} source;

	union {
		struct {
			char *path;
		} import;

		struct {
			char *name;
		} global;

		struct {
			char *name;

			unsigned number_of_arguments;
			char **argument_names;

			ast_block *body;
		} function;
	};
};

ast_declaration *next_declaration(tokenizer *tzr);

void free_ast_primary(ast_primary *primary);
void free_ast_expression(ast_expression *expression);
void free_ast_statement(ast_statement *statement);
void free_ast_block(ast_block *block);
void free_ast_declaration(ast_declaration *declaration);

void dump_ast_primary(FILE *out, const ast_primary *primary);
void dump_ast_expression(FILE *out, const ast_expression *expression);
void dump_ast_statement(FILE *out, const ast_statement *statement, unsigned indent);
void dump_ast_block(FILE *out, const ast_block *block, unsigned indent);
void dump_ast_declaration(FILE *out, const ast_declaration *declaration);
