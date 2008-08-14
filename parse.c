#include <stdio.h>
#include "parse.h"


int parse(char *filename)
{
	int result;
	FILE *file = fopen(filename, "r");
	yyrestart(file);
	result = yyparse();
	fclose(file);
	reset_tokenizer();
	return result;
}
