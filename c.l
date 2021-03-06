D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <string.h>
#include "parse.h"

static void count();
static void ignore();
static int check_type();
%}

%%
"/*"([^\*]|\*[^/])*"*/"	{ ignore(); }
"//"[^\n]*		{ ignore(); }
"_Bool"			{ count(); return BOOL; }
"_Complex"		{ count(); return COMPLEX; }
"_Imaginary"		{ count(); return IMAGINARY; }
"auto"			{ count(); return AUTO; }
"break"			{ count(); return BREAK; }
"case"			{ count(); return CASE; }
"char"			{ count(); return CHAR; }
"const"			{ count(); return CONST; }
"continue"		{ count(); return CONTINUE; }
"default"		{ count(); return DEFAULT; }
"do"			{ count(); return DO; }
"double"		{ count(); return DOUBLE; }
"else"			{ count(); return ELSE; }
"enum"			{ count(); return ENUM; }
"extern"		{ count(); return EXTERN; }
"float"			{ count(); return FLOAT; }
"for"			{ count(); return FOR; }
"goto"			{ count(); return GOTO; }
"if"			{ count(); return IF; }
"inline"		{ count(); return INLINE; }
"int"			{ count(); return INT; }
"long"			{ count(); return LONG; }
"register"		{ count(); return REGISTER; }
"restrict"		{ count(); return RESTRICT; }
"return"		{ count(); return RETURN; }
"short"			{ count(); return SHORT; }
"signed"		{ count(); return SIGNED; }
"sizeof"		{ count(); return SIZEOF; }
"static"		{ count(); return STATIC; }
"struct"		{ count(); return STRUCT; }
"switch"		{ count(); return SWITCH; }
"typedef"		{ count(); return TYPEDEF; }
"union"			{ count(); return UNION; }
"unsigned"		{ count(); return UNSIGNED; }
"void"			{ count(); return VOID; }
"volatile"		{ count(); return VOLATILE; }
"while"			{ count(); return WHILE; }

{L}({L}|{D})*		{ count(); return check_type(); }

0[xX]{H}+{IS}?		{ count(); return CONSTANT; }
0{D}+{IS}?		{ count(); return CONSTANT; }
{D}+{IS}?		{ count(); return CONSTANT; }
'(\\.|[^\\'])+'		{ count(); return CONSTANT; }

{D}+{E}{FS}?		{ count(); return CONSTANT; }
{D}*"."{D}+({E})?{FS}?	{ count(); return CONSTANT; }
{D}+"."{D}*({E})?{FS}?	{ count(); return CONSTANT; }

\"(\\.|[^\\"])*\"	{ count(); return STRING_LITERAL; }
#([^\n]|\\\n)+		{ count(); return MACRO; }

">>="			{ count(); return RIGHT_ASSIGN; }
"<<="			{ count(); return LEFT_ASSIGN; }
"+="			{ count(); return ADD_ASSIGN; }
"-="			{ count(); return SUB_ASSIGN; }
"*="			{ count(); return MUL_ASSIGN; }
"/="			{ count(); return DIV_ASSIGN; }
"%="			{ count(); return MOD_ASSIGN; }
"&="			{ count(); return AND_ASSIGN; }
"^="			{ count(); return XOR_ASSIGN; }
"|="			{ count(); return OR_ASSIGN; }
">>"			{ count(); return RIGHT_OP; }
"<<"			{ count(); return LEFT_OP; }
"++"			{ count(); return INC_OP; }
"--"			{ count(); return DEC_OP; }
"->"			{ count(); return PTR_OP; }
"&&"			{ count(); return AND_OP; }
"||"			{ count(); return OR_OP; }
"<="			{ count(); return LE_OP; }
">="			{ count(); return GE_OP; }
"=="			{ count(); return EQ_OP; }
"!="			{ count(); return NE_OP; }
";"			{ count(); return ';'; }
"{"			{ count(); return '{'; }
"}"			{ count(); return '}'; }
","			{ count(); return ','; }
":"			{ count(); return ':'; }
"="			{ count(); return '='; }
"("			{ count(); return '('; }
")"			{ count(); return ')'; }
"["			{ count(); return '['; }
"]"			{ count(); return ']'; }
"."			{ count(); return '.'; }
"&"			{ count(); return '&'; }
"!"			{ count(); return '!'; }
"~"			{ count(); return '~'; }
"-"			{ count(); return '-'; }
"+"			{ count(); return '+'; }
"*"			{ count(); return '*'; }
"/"			{ count(); return '/'; }
"%"			{ count(); return '%'; }
"<"			{ count(); return '<'; }
">"			{ count(); return '>'; }
"^"			{ count(); return '^'; }
"|"			{ count(); return '|'; }
"?"			{ count(); return '?'; }

[ \t\v\n\f]		{ ignore(); }
.			{ ignore(); }

%%

int yywrap()
{
	return 1;
}

#define MAXTOKLEN	4 * 1024

long token_offset = 0;
static char token_str[MAXTOKLEN];
static char *cur_filename = NULL;

static void count()
{
	yylloc.start = token_offset;
	token_offset += strlen(yytext);
	strcpy(token_str, yytext);
	yylval = token_str;
	yylloc.end = token_offset;
}

static void ignore()
{
	token_offset += strlen(yytext);
}

static int check_type()
{
	return typedef_name(cur_filename, yytext, yylloc.start) ?
		TYPE_NAME : IDENTIFIER;
}

void reset_tokenizer(char *filename)
{
	token_offset = 0;
	cur_filename = filename;
}
