#ifndef _AST_H
#define _AST_H

enum nodetype {
	AST_STRING,
	AST_INTEGER,
	AST_IDENTIFIER,

	AST_UNARY,
	AST_CAST,
	AST_SIZEOF,

	AST_BINARY,
	AST_CONDITIONAL,

	AST_GETINDEX,
	AST_GETATTR,
	AST_DEREF,
	AST_CALL,

	AST_ASSIGN,
	AST_MACRO,
	AST_GOTO,
	AST_CONTINUE,
	AST_BREAK,
	AST_RETURN,

	AST_FOR,
	AST_DO,
	AST_WHILE,
	AST_IF,
	AST_SWITCH,
	AST_BLOCK,

	AST_LABELED,
	AST_CASED,
	AST_DEFAULTED,

	AST_EXPRLIST,
	AST_DECL,
	AST_DECLLIST,
	AST_DECLSPEC,
	AST_INITLIST,
	AST_INIT,
	AST_ARGLIST,

	AST_PARAMDECL,
	AST_PARAMLIST,

	AST_PTR,
	AST_SPECLIST,

	AST_TYPE,
	AST_STMTLIST,
	AST_STMT,
	AST_STORAGECLASS,
	AST_UNARYOP,

	AST_INITIALIZERLIST,
	AST_INITIALIZER,
	AST_ADECL,
	AST_ATYPE,
	AST_ARGDECL,

	AST_STRUCT,
	AST_STRUCTDECL,
	AST_STRUCTLIST,
	AST_STRUCTBITS,

	AST_IDLIST,

	AST_ENUM,
	AST_ENUMLIST,
	AST_ENUMVAL,

	AST_FILE,
	AST_FUNCTION,
	AST_BODY,
};

struct node {
	enum nodetype type;
	long start, end;
	struct node **children;
	int count;
	void *data;
};

struct node *parse(char *filename);
void free_node(struct node* node);
void node_at(struct node* node, long offset);

#endif
