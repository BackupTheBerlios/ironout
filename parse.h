#ifndef _PARSE_H
#define _PARSE_H

#define YYLTYPE struct yyltype
#define YYSTYPE char*
#define YYERROR_VERBOSE	1
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
void yyrestart(FILE *file);
int yyparse();

void reset_tokenizer(char *filename);

#include "cyacc.h"
#include "ast.h"

struct node *push_node(enum nodetype type, long start, long end, int nchild);
struct node *push_node_name(enum nodetype type, long start, long end, char *name);
struct node *push_decl(enum nodetype type, long start, long end,
		       int nchild, enum decltype decltype);

int typedef_name(char *filename, char *name, long offset);

#endif
