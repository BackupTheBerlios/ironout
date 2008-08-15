#ifndef _PARSE_H
#define _PARSE_H

#define YYLTYPE struct yyltype
#define YYSTYPE char*
#define YYLLOC_DEFAULT(Current, Rhs, N)					\
	do								\
		if (0 && N) {						\
			(Current).start = YYRHSLOC(Rhs, 1).start;	\
			(Current).end = YYRHSLOC(Rhs, N).end;		\
		} else {						\
			last_token_location(N, &(Current).start, &(Current).end); \
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

void last_token_location(int count, long *start, long *end);
void reset_tokenizer();

#include "gram.tab.h"
#include "ast.h"

struct node *parse(char *filename);
void push_node(enum nodetype type, long start, long end, int nchild);
void free_node(struct node* node);

#endif
