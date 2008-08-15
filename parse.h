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
			printf("WARNING: couldn't find location!\n");	\
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

void push_node(enum nodetype type, long start, long end, int nchild);

#endif
