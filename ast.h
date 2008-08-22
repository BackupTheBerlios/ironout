#ifndef _AST_H
#define _AST_H

enum nodetype {
	AST_ADECL,
	AST_ADECL2,
	AST_ARGDECL,
	AST_ARGLIST,
	AST_ASSIGN,
	AST_ATYPE,
	AST_BINARY,
	AST_BLOCK,
	AST_BODY,
	AST_BREAK,
	AST_CALL,
	AST_CASED,
	AST_CAST,
	AST_CONDITIONAL,
	AST_CONTINUE,
	AST_DECL,
	AST_DECL2,
	AST_DECLLIST,
	AST_DECLSPEC,
	AST_DECLSTMT,
	AST_DEFAULTED,
	AST_DEREF,
	AST_DO,
	AST_ENUM,
	AST_ENUMLIST,
	AST_ENUMVAL,
	AST_EXPRLIST,
	AST_FILE,
	AST_FOR,
	AST_FUNCTION,
	AST_GETATTR,
	AST_GETINDEX,
	AST_GOTO,
	AST_IDENTIFIER,
	AST_IDLIST,
	AST_IF,
	AST_INIT,
	AST_INITIALIZER,
	AST_INITIALIZERLIST,
	AST_INITLIST,
	AST_INTEGER,
	AST_LABELED,
	AST_MACRO,
	AST_PARAMDECL,
	AST_PARAMLIST,
	AST_PTR,
	AST_RETURN,
	AST_SIZEOF,
	AST_SPECLIST,
	AST_STMT,
	AST_STMTLIST,
	AST_STORAGECLASS,
	AST_STRING,
	AST_STRUCT,
	AST_STRUCTBITS,
	AST_STRUCTDECL,
	AST_STRUCTKW,
	AST_STRUCTLIST,
	AST_SWITCH,
	AST_TYPE,
	AST_TYPENAME,
	AST_UNARY,
	AST_UNARYOP,
	AST_UNIONKW,
	AST_WHILE
};

struct node {
	enum nodetype type;
	long start, end;
	struct node **children;
	int count;
	void *data;
};

struct node *parse(char *filename);
void node_free(struct node *node);
struct node *node_find(struct node *node, long offset);
void node_walk(struct node *, int (*) (struct node *, void *), void *);
int node_cmp(struct node *n1, struct node *n2);

#endif
