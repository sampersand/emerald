#include "ast.h"
#include "token.h"
#include "value.h"
#include <stdlib.h>
#include <assert.h>

#define parse_error(tzr, ...) ( \
	fprintf(stderr, "parse error at line %d: ", tzr->line_number), \
	fprintf(stderr, __VA_ARGS__), \
	exit(1))

static token peek(tokenizer *tzr) {
	if (tzr->prev.kind == TOKEN_KIND_UNDEFINED)
		tzr->prev = next_token(tzr);

	return tzr->prev;
}

static token advance(tokenizer *tzr) {
	token tkn = peek(tzr);
	tzr->prev.kind = TOKEN_KIND_UNDEFINED;
	return tkn;
}

static void unadvance(tokenizer *tzr, token tkn) {
	assert(tzr->prev.kind == TOKEN_KIND_UNDEFINED);
	tzr->prev = tkn;
}

static bool guard(tokenizer *tzr, token_kind kind) {
	if (peek(tzr).kind == kind) {
		advance(tzr);
		return true;
	}

	return false;
}

static char *expect_identifier(tokenizer *tzr, const char *whence) {
	token tkn = advance(tzr);

	if (tkn.kind != TOKEN_KIND_IDENTIFIER)
		parse_error(tzr, "expected identifier %s", whence);

	return tkn.identifier;
}

static ast_expression *parse_expression(tokenizer *tzr);
static ast_primary *parse_primary(tokenizer *tzr) {
	ast_primary *primary = xmalloc(sizeof(ast_primary));
	token tkn = advance(tzr);

	// Parse the initial primary
	switch (tkn.kind) {
	case TOKEN_KIND_LPAREN:
		primary->kind = AST_PRIMARY_PAREN;

		primary->paren.expression = parse_expression(tzr);
		if (primary->paren.expression == NULL)
			parse_error(tzr, "expected an expression after `(`");

		if (!guard(tzr, TOKEN_KIND_RPAREN))
			parse_error(tzr, "expected `)` after expression within `(...)`");

		break;

	case TOKEN_KIND_LBRACKET:
		primary->kind = AST_PRIMARY_ARRAY_LITERAL;

		unsigned capacity = 4;
		primary->array_literal.length = 0;
		primary->array_literal.elements = xmalloc(capacity * sizeof(ast_expression *));

		while (!guard(tzr, TOKEN_KIND_RBRACKET)) {
			if (primary->array_literal.length == capacity) {
				capacity *= 2;
				primary->array_literal.elements =
					xrealloc(primary->array_literal.elements, capacity * sizeof(ast_expression *));
			}

			primary->array_literal.elements[primary->array_literal.length] = parse_expression(tzr);
			if (primary->array_literal.elements[primary->array_literal.length] == NULL)
				parse_error(tzr, "expected an expression within array literal");

			primary->array_literal.length++;

			if (!guard(tzr, TOKEN_KIND_COMMA)) {
				if (!guard(tzr, TOKEN_KIND_RBRACKET))
					parse_error(tzr, "expected either an `,` or `]` after array element literal");
				break;
			}
		}
		break;

	case TOKEN_KIND_SUBTRACT:
	case TOKEN_KIND_NOT:
		primary->kind = AST_PRIMARY_UNARY_OPERATOR;
		primary->unary_operator.operator = tkn.kind == TOKEN_KIND_NOT ? UNARY_OP_NOT : UNARY_OP_NEGATE;
		primary->unary_operator.primary = parse_primary(tzr);

		if (primary->unary_operator.primary == NULL)
			parse_error(tzr, "expected a primary after `-` / `!`");
		break;

	case TOKEN_KIND_IDENTIFIER:
		primary->kind = AST_PRIMARY_VARIABLE;
		primary->variable.name = tkn.identifier;
		break;

	case TOKEN_KIND_LITERAL:
		primary->kind = AST_PRIMARY_LITERAL;
		primary->literal.val = tkn.val;
		break;

	default:
		unadvance(tzr, tkn);
		free(primary);
		return NULL;
	}

	// Parse subsequent indexing or function calls.

	while (true) {
		ast_primary *temp_primary;
		tkn = advance(tzr);

		switch (tkn.kind) {
		case TOKEN_KIND_LBRACKET:
			temp_primary = xmalloc(sizeof(ast_primary));
			temp_primary->kind = AST_PRIMARY_INDEX;
			temp_primary->index.source = primary;
			primary = temp_primary;

			primary->index.index = parse_expression(tzr);

			if (primary->index.index == NULL)
				parse_error(tzr, "expected an expression for indexing");

			if (!guard(tzr, TOKEN_KIND_RBRACKET))
				parse_error(tzr, "expected an `]` after  indexing");

			break;

		case TOKEN_KIND_LPAREN:
			temp_primary = xmalloc(sizeof(ast_primary));
			temp_primary->kind = AST_PRIMARY_FUNCTION_CALL;
			temp_primary->function_call.function = primary;
			primary = temp_primary;

			unsigned capacity = 4;
			primary->function_call.number_of_arguments = 0;
			primary->function_call.arguments = xmalloc(capacity * sizeof(ast_expression *));

			while (!guard(tzr, TOKEN_KIND_RPAREN)) {
				if (primary->function_call.number_of_arguments == capacity) {
					capacity *= 2;
					primary->function_call.arguments =
						xrealloc(primary->function_call.arguments, capacity * sizeof(ast_expression *));
				}

				primary->function_call.arguments[primary->function_call.number_of_arguments] =
					parse_expression(tzr);
				if (primary->function_call.arguments[primary->function_call.number_of_arguments] == NULL)
					parse_error(tzr, "expected an expression for argument calls");
				primary->function_call.number_of_arguments++;

				if (!guard(tzr, TOKEN_KIND_COMMA)) {
					if (!guard(tzr, TOKEN_KIND_RPAREN))
						parse_error(tzr, "expected either an `,` or `)` after function call argument");
					break;
				}
			}
			break;

		default:
			unadvance(tzr, tkn);
			return primary;
		}
	}
}

static binary_operator binary_operator_from_token_kind(token_kind kind) {
	switch (kind) {
	case TOKEN_KIND_ASSIGN:                return BINARY_OP_UNDEF;
	case TOKEN_KIND_ADD_ASSIGN:            return BINARY_OP_ADD;
	case TOKEN_KIND_SUBTRACT_ASSIGN:       return BINARY_OP_SUBTRACT;
	case TOKEN_KIND_MULTIPLY_ASSIGN:       return BINARY_OP_MULTIPLY;
	case TOKEN_KIND_DIVIDE_ASSIGN:         return BINARY_OP_DIVIDE;
	case TOKEN_KIND_MODULO_ASSIGN:         return BINARY_OP_MODULO;
	case TOKEN_KIND_ADD:                   return BINARY_OP_ADD;
	case TOKEN_KIND_SUBTRACT:              return BINARY_OP_SUBTRACT;
	case TOKEN_KIND_MULTIPLY:              return BINARY_OP_MULTIPLY;
	case TOKEN_KIND_DIVIDE:                return BINARY_OP_DIVIDE;
	case TOKEN_KIND_MODULO:                return BINARY_OP_MODULO;
	case TOKEN_KIND_EQUAL:                 return BINARY_OP_EQUAL;
	case TOKEN_KIND_NOT_EQUAL:             return BINARY_OP_NOT_EQUAL;
	case TOKEN_KIND_LESS_THAN:             return BINARY_OP_LESS_THAN;
	case TOKEN_KIND_LESS_THAN_OR_EQUAL:    return BINARY_OP_LESS_THAN_OR_EQUAL;
	case TOKEN_KIND_GREATER_THAN:          return BINARY_OP_GREATER_THAN;
	case TOKEN_KIND_GREATER_THAN_OR_EQUAL: return BINARY_OP_GREATER_THAN_OR_EQUAL;
	default: bug("invalid operator kind %d", kind);
	}
}

static ast_expression *parse_expression(tokenizer *tzr) {
	ast_primary *primary = parse_primary(tzr);
	if (primary == NULL)
		return NULL;

	ast_expression *expression = xmalloc(sizeof(ast_expression));
	token tkn = advance(tzr);

	switch (tkn.kind) {
	case TOKEN_KIND_ADD_ASSIGN:
	case TOKEN_KIND_SUBTRACT_ASSIGN:
	case TOKEN_KIND_MULTIPLY_ASSIGN:
	case TOKEN_KIND_DIVIDE_ASSIGN:
	case TOKEN_KIND_MODULO_ASSIGN:
	case TOKEN_KIND_ASSIGN: {
		ast_expression *rhs = parse_expression(tzr);
		if (rhs == NULL)
			parse_error(tzr, "expected an expression after `=`");

		switch (primary->kind) {
		case AST_PRIMARY_VARIABLE:
			expression->kind = AST_EXPRESSION_ASSIGN;
			expression->assign.name = primary->variable.name;
			expression->assign.value = rhs;
			expression->assign.operator = binary_operator_from_token_kind(tkn.kind);
			break;

		case AST_PRIMARY_INDEX:
			expression->kind = AST_EXPRESSION_INDEX_ASSIGN;
			expression->index_assign.source = primary->index.source;
			expression->index_assign.index = primary->index.index;
			expression->index_assign.value = rhs;
			expression->index_assign.operator = binary_operator_from_token_kind(tkn.kind);
			break;

		default:
			parse_error(tzr, "you may online assign to identifiers and array indexes");
		}

		free(primary);
		break;
	}

	case TOKEN_KIND_AND_AND:
	case TOKEN_KIND_OR_OR:
		expression->kind = AST_EXPRESSION_SHORT_CIRCUIT_OPERATOR;
		expression->short_circuit_operator.operator =
			tkn.kind == TOKEN_KIND_AND_AND ? SHORT_CIRCUIT_AND_AND : SHORT_CIRCUIT_OR_OR;
		expression->short_circuit_operator.lhs = primary;
		expression->short_circuit_operator.rhs = parse_expression(tzr);

		if (expression->short_circuit_operator.rhs == NULL)
			parse_error(tzr, "expected RHS after `&&` / `||`");

		break;

	case TOKEN_KIND_ADD:
	case TOKEN_KIND_SUBTRACT:
	case TOKEN_KIND_MULTIPLY:
	case TOKEN_KIND_DIVIDE:
	case TOKEN_KIND_MODULO:
	case TOKEN_KIND_LESS_THAN:
	case TOKEN_KIND_GREATER_THAN:
	case TOKEN_KIND_LESS_THAN_OR_EQUAL:
	case TOKEN_KIND_GREATER_THAN_OR_EQUAL:
	case TOKEN_KIND_EQUAL:
	case TOKEN_KIND_NOT_EQUAL:
		expression->kind = AST_EXPRESSION_BINARY_OPERATOR;
		expression->binary_operator.operator = binary_operator_from_token_kind(tkn.kind);
		expression->binary_operator.lhs = primary;
		expression->binary_operator.rhs = parse_expression(tzr);

		if (expression->binary_operator.rhs == NULL)
			parse_error(tzr, "expected RHS after binary operator");

		break;

	default:
		unadvance(tzr, tkn);

		expression->kind = AST_EXPRESSION_PRIMARY;
		expression->primary = primary;
		break;
	}

	return expression;
}

static ast_block *parse_block(tokenizer *tzr);

static ast_statement *parse_statement(tokenizer *tzr) {
	ast_statement *statement = xmalloc(sizeof(ast_statement));
	token tkn = advance(tzr);

	switch (tkn.kind) {
	case TOKEN_KIND_LOCAL:
		statement->kind = AST_STATEMENT_LOCAL;
		statement->local.name = expect_identifier(tzr, "local name");

		if (guard(tzr, TOKEN_KIND_ASSIGN)) {
			statement->local.initializer = parse_expression(tzr);
			if (statement->local.initializer == NULL)
				parse_error(tzr, "expected expression after `=` in local declaration");
		} else {
			statement->local.initializer = NULL;
		}

		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `local`");
		break;

	case TOKEN_KIND_RETURN:
		statement->kind = AST_STATEMENT_RETURN;
		statement->return_.expression = parse_expression(tzr); // note this can be null.

		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `return`");
		break;

	case TOKEN_KIND_CONTINUE:
		statement->kind = AST_STATEMENT_CONTINUE;
		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `continue`");
		break;

	case TOKEN_KIND_BREAK:
		statement->kind = AST_STATEMENT_BREAK;
		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `break`");
		break;

	case TOKEN_KIND_WHILE:
		statement->kind = AST_STATEMENT_WHILE;
		statement->while_.condition = parse_expression(tzr);
		if (statement->while_.condition == NULL)
			parse_error(tzr, "expected condition for `while`");

		statement->while_.body = parse_block(tzr);
		if (statement->while_.body == NULL)
			parse_error(tzr, "expected body for `while`");
		break;

	case TOKEN_KIND_IF:
		statement->kind = AST_STATEMENT_IF;
		statement->if_.condition = parse_expression(tzr);
		if (statement->if_.condition == NULL)
			parse_error(tzr, "expected condition for `if`");

		statement->if_.if_true = parse_block(tzr);
		if (statement->if_.if_true == NULL)
			parse_error(tzr, "expected body for `if`");

		if (guard(tzr, TOKEN_KIND_ELSE)) {
			statement->if_.if_false = parse_block(tzr);
			if (statement->if_.if_false == NULL)
				parse_error(tzr, "expected body for `else`");
		} else {
			statement->if_.if_false = NULL;
		}

		break;

	default:
		unadvance(tzr, tkn);
		statement->kind = AST_STATEMENT_EXPRESSION;
		statement->expression = parse_expression(tzr);
		if (statement->expression == NULL) {
			free(statement);
			return NULL;
		}

		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after expression");
		break;
	}

	return statement;
}

static ast_block *parse_block(tokenizer *tzr) {
	if (!guard(tzr, TOKEN_KIND_LBRACE))
		return NULL;

	ast_block *block = xmalloc(sizeof(ast_block));

	unsigned capacity = 4;
	block->number_of_statements = 0;
	block->statements = xmalloc(capacity * sizeof(ast_statement *));

	while (!guard(tzr, TOKEN_KIND_RBRACE)) {
		while (guard(tzr, TOKEN_KIND_SEMICOLON)) {
			// eat random leading semicolons.
		}

		ast_statement *statement = parse_statement(tzr);
		if (statement == NULL) {
			if (!guard(tzr, TOKEN_KIND_RBRACKET))
				parse_error(tzr, "expected `}` at end of block body");
			break;
		}

		if (capacity == block->number_of_statements) {
			capacity *= 2;
			block->statements = xrealloc(block->statements, capacity * sizeof(ast_statement *));
		}

		block->statements[block->number_of_statements] = statement;
		block->number_of_statements++;
	}

	return block;
}

ast_declaration *next_declaration(tokenizer *tzr) {
	ast_declaration *declaration = xmalloc(sizeof(ast_declaration));
	token tkn = advance(tzr);

	declaration->source.line_number = tzr->line_number;
	declaration->source.filename = tzr->filename;

	switch (tkn.kind) {
	case TOKEN_KIND_GLOBAL:
		declaration->kind = AST_DECLARATION_GLOBAL;
		declaration->global.name = expect_identifier(tzr, "global name");

		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `global` declaration");
		break;

	case TOKEN_KIND_IMPORT: {
		declaration->kind = AST_DECLARATION_IMPORT;

		token path_token = advance(tzr);
		if (path_token.kind != TOKEN_KIND_LITERAL || !is_string(path_token.val))
			parse_error(tzr, "`import` only takes strings");
		if (!guard(tzr, TOKEN_KIND_SEMICOLON))
			parse_error(tzr, "expected `;` after `import` declaration");


		// `read_file` expects a nul terminated string, but `string`s are not nul terminated.
		// So we have to make one.
		string *path_string = as_string(path_token.val);
		char *path = new_cstr_from_string(path_string);
		free_string(path_string);

		if (path == NULL)
			parse_error(tzr, "import paths must not contain `\\0`");
		break;
	}

	case TOKEN_KIND_FUNCTION:
		declaration->kind = AST_DECLARATION_FUNCTION;
		declaration->function.name = expect_identifier(tzr, "function name");

		if (!guard(tzr, TOKEN_KIND_LPAREN))
			parse_error(tzr, "expected `(` after function name");

		unsigned capacity = 4;
		declaration->function.number_of_arguments = 0;
		declaration->function.argument_names = xmalloc(capacity * sizeof(char *));

		while (!guard(tzr, TOKEN_KIND_RPAREN)) {
			if (declaration->function.number_of_arguments == capacity) {
				capacity *= 2;
				declaration->function.argument_names =
					xrealloc(declaration->function.argument_names, capacity * sizeof(char *));
			}

			declaration->function.argument_names[declaration->function.number_of_arguments] =
				expect_identifier(tzr, "function argument");
			declaration->function.number_of_arguments++;

			if (!guard(tzr, TOKEN_KIND_COMMA)) {
				if (!guard(tzr, TOKEN_KIND_RPAREN))
					parse_error(tzr, "expected `,` or `)` after argument name");
				break;
			}
		}

		declaration->function.body = parse_block(tzr);
		if (declaration->function.body == NULL)
			parse_error(tzr, "expected body for function %s", declaration->function.name);

		break;

	case TOKEN_KIND_UNDEFINED:
		free(declaration);
		return NULL;

	default:
		fprintf(stderr, "parse error at line %d: unexpected token ", tzr->line_number);
		dump_token(stderr, tkn);
		exit(1);
	}

	return declaration;
}

void dump_ast_primary(FILE *out, const ast_primary *primary) {
	switch (primary->kind) {
	case AST_PRIMARY_PAREN:
		fputc('(', out);
		dump_ast_expression(out, primary->paren.expression);
		fputc(')', out);
		break;

	case AST_PRIMARY_INDEX:
		dump_ast_primary(out, primary->index.source);
		fputc('[', out);
		dump_ast_expression(out, primary->index.index);
		fputc(']', out);
		break;

	case AST_PRIMARY_FUNCTION_CALL:
		dump_ast_primary(out, primary->function_call.function);
		fputc('(', out);
		for (unsigned i = 0; i < primary->function_call.number_of_arguments; i++) {
			if (i != 0)
				fputs(", ", out);

			dump_ast_expression(out, primary->function_call.arguments[i]);
		}
		fputc(')', out);
		break;

	case AST_PRIMARY_UNARY_OPERATOR:
		switch (primary->unary_operator.operator) {
		case UNARY_OP_NEGATE: fputc('-', out); break;
		case UNARY_OP_NOT:    fputc('!', out); break;
		}
		dump_ast_primary(out, primary->unary_operator.primary);
		break;

	case AST_PRIMARY_ARRAY_LITERAL:
		fputc('[', out);
		for (unsigned i = 0; i < primary->array_literal.length; i++) {
			if (i != 0)
				fputs(", ", out);

			dump_ast_expression(out, primary->array_literal.elements[i]);
		}
		fputc(']', out);
		break;

	case AST_PRIMARY_VARIABLE:
		fputs(primary->variable.name, out);
		break;

	case AST_PRIMARY_LITERAL:
		dump_value(out, primary->literal.val);
		break;
	}
}

static const char *binary_operator_to_string(binary_operator op) {
	switch (op) {
	case BINARY_OP_UNDEF: return "=";
	case BINARY_OP_ADD: return "+";
	case BINARY_OP_SUBTRACT: return "-";
	case BINARY_OP_MULTIPLY: return "*";
	case BINARY_OP_DIVIDE: return "/";
	case BINARY_OP_MODULO: return "%";
	case BINARY_OP_EQUAL: return "==";
	case BINARY_OP_NOT_EQUAL: return "!=";
	case BINARY_OP_LESS_THAN: return "<";
	case BINARY_OP_LESS_THAN_OR_EQUAL: return "<=";
	case BINARY_OP_GREATER_THAN: return ">";
	case BINARY_OP_GREATER_THAN_OR_EQUAL: return ">=";
	}
}

void dump_ast_expression(FILE *out, const ast_expression *expression) {
	switch (expression->kind) {
	case AST_EXPRESSION_ASSIGN:
		fputs(expression->assign.name, out);
		fprintf(out, " %s ", binary_operator_to_string(expression->assign.operator));
		dump_ast_expression(out, expression->assign.value);
		break;

	case AST_EXPRESSION_INDEX_ASSIGN:
		dump_ast_primary(out, expression->index_assign.source);
		fputc('[', out);
		dump_ast_expression(out, expression->index_assign.index);
		fprintf(out, "] %s = ", binary_operator_to_string(expression->index_assign.operator));
		dump_ast_expression(out, expression->index_assign.value);
		break;

	case AST_EXPRESSION_SHORT_CIRCUIT_OPERATOR:
		dump_ast_primary(out, expression->short_circuit_operator.lhs);
		if (expression->short_circuit_operator.operator == SHORT_CIRCUIT_OR_OR) {
			fputs(" || ", out);
		} else {
			fputs(" && ", out);
		}
		dump_ast_expression(out, expression->short_circuit_operator.rhs);
		break;

	case AST_EXPRESSION_BINARY_OPERATOR:
		dump_ast_primary(out, expression->binary_operator.lhs);
		fprintf(out, " %s ", binary_operator_to_string(expression->binary_operator.operator));
		dump_ast_expression(out, expression->binary_operator.rhs);
		break;

	case AST_EXPRESSION_PRIMARY:
		dump_ast_primary(out, expression->primary);
		break;
	}
}

void dump_ast_statement(FILE *out, const ast_statement *statement, unsigned indent) {
	switch (statement->kind) {
	case AST_STATEMENT_LOCAL:
		fprintf(out, "local %s", statement->local.name);
		if (statement->local.initializer != NULL) {
			fputs(" = ", out);
			dump_ast_expression(out, statement->local.initializer);
		}
		fputc(';', out);
		break;

	case AST_STATEMENT_RETURN:
		fputs("return", out);
		if (statement->return_.expression != NULL) {
			fputc(' ', out);
			dump_ast_expression(out, statement->return_.expression);
		}
		fputc(';', out);
		break;

	case AST_STATEMENT_IF:
		fputs("if ", out);
		dump_ast_expression(out, statement->if_.condition);
		fputc(' ', out);
		dump_ast_block(out, statement->if_.if_true, indent + 1);
		if (statement->if_.if_false != NULL) {
			fputs(" else ", out);
			dump_ast_block(out, statement->if_.if_false, indent + 1);
		}
		break;

	case AST_STATEMENT_WHILE:
		fputs("while ", out);
		dump_ast_expression(out, statement->while_.condition);
		fputc(' ', out);
		dump_ast_block(out, statement->while_.body, indent + 1);
		break;

	case AST_STATEMENT_BREAK:
		fputs("break;", out);
		break;

	case AST_STATEMENT_CONTINUE:
		fputs("continue;", out);
		break;

	case AST_STATEMENT_EXPRESSION:
		dump_ast_expression(out, statement->expression);
	}
}

static void print_indent(FILE *out, unsigned indent) {
	for (unsigned i = 0; i < indent; i++)
		fputs("   ", out);
}

void dump_ast_block(FILE *out, const ast_block *block, unsigned indent) {
	fputs("{\n", out);
	for (unsigned i = 0; i < block->number_of_statements; i++) {
		print_indent(out, indent);
		dump_ast_statement(out, block->statements[i], indent + 1);
		fputc('\n', out);
	}
	print_indent(out, indent - 1);
	fputs("}\n", out);
}

void dump_ast_declaration(FILE *out, const ast_declaration *declaration) {
	switch (declaration->kind) {
	case AST_DECLARATION_GLOBAL:
		fprintf(out, "global %s;\n", declaration->global.name);
		break;
	case AST_DECLARATION_IMPORT:
		// this won't escape special characters, but eh, it's debug code, so whatever.
		fprintf(out, "import \"%s\";\n", declaration->import.path);
		break;
	case AST_DECLARATION_FUNCTION:
		fprintf(out, "function %s(", declaration->function.name);

		for (unsigned i = 0; i < declaration->function.number_of_arguments; i++) {
			if (i != 0)
				fputs(", ", out);

			fputs(declaration->function.argument_names[i], out);
		}

		fputs(") ", out);
		dump_ast_block(out, declaration->function.body, 1);
	}
}
