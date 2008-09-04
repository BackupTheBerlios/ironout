#ifndef _PARSE_H
#define _PARSE_H

#define YYLTYPE struct yyltype
#define YYSTYPE char*
#define YYLLOC_DEFAULT(Current, Rhs, N)					\
	do								\
		if (N) {						\
			(Current).start = YYRHSLOC(Rhs, 1).start;	\
			(Current).end = YYRHSLOC(Rhs, N).end;		\
		} else {						\
			fprintf(stderr, "WARNING: couldn't find location!\n"); \
		}							\
	while (0)

struct yyltype {
	long start;
	long end;
};

void yyerror(char *s);
int yylex();
void yyrestart(FILE *filename);
int yyparse();

void reset_tokenizer();

#include "cyacc.h"
#include "ast.h"

struct node *push_node(enum nodetype type, long start, long end, int nchild);
struct node *push_node_name(enum nodetype type, long start, long end, char *name);

int is_typename(char *name);

#endif
