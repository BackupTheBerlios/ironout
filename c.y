%{
#include <stdio.h>
#include "ast.h"
#include "parse.h"

%}
%token IDENTIFIER CONSTANT STRING_LITERAL MACRO SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN INLINE STATIC AUTO REGISTER RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELIPSIS RANGE

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start translation_unit
%%

primary_expr
	: identifier
	| CONSTANT
		{ push_node(AST_INTEGER, @$.start, @$.end, 0); }
	| joined_string
		{ push_node(AST_STRING, @$.start, @$.end, 0); }
	| '(' expr ')'
	;

joined_string
	: joined_string STRING_LITERAL
	| STRING_LITERAL
	;

postfix_expr
	: primary_expr
	| postfix_expr '[' expr ']'
		{ push_node(AST_GETINDEX, @$.start, @$.end, 2); }
	| postfix_expr '(' ')'
		{ push_node(AST_CALL, @$.start, @$.end, 1); }
	| postfix_expr '(' argument_expr_list ')'
		{ push_node(AST_CALL, @$.start, @$.end, 2); }
	| postfix_expr '.' identifier
		{ push_node(AST_GETATTR, @$.start, @$.end, 2); }
	| postfix_expr PTR_OP identifier
		{ push_node(AST_DEREF, @$.start, @$.end, 2); }
	| postfix_expr INC_OP
		{ push_node(AST_UNARY, @$.start, @$.end, 1); }
	| postfix_expr DEC_OP
		{ push_node(AST_UNARY, @$.start, @$.end, 1); }
	| '(' type_name ')' '{' initializer_list '}'
		{ push_node(AST_C99INIT, @$.start, @$.end, 2); }
	| '(' type_name ')' '{' initializer_list ',' '}'
		{ push_node(AST_C99INIT, @$.start, @$.end, 2); }
	;

argument_expr_list
	: assignment_expr
		{ push_node(AST_ARGLIST, @$.start, @$.end, 1); }
	| argument_expr_list ',' assignment_expr
		{ push_node(AST_ARGLIST, @$.start, @$.end, 2); }
	;

unary_expr
	: postfix_expr
	| INC_OP unary_expr
		{ push_node(AST_UNARY, @$.start, @$.end, 1); }
	| DEC_OP unary_expr
		{ push_node(AST_UNARY, @$.start, @$.end, 1); }
	| unary_operator cast_expr
		{ push_node(AST_UNARY, @$.start, @$.end, 2); }
	| SIZEOF unary_expr
		{ push_node(AST_SIZEOF, @$.start, @$.end, 1); }
	| SIZEOF '(' type_name ')'
		{ push_node(AST_SIZEOF, @$.start, @$.end, 1); }
	;

unary_operator
	: '&'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	| '*'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	| '+'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	| '-'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	| '~'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	| '!'
		{ push_node(AST_UNARYOP, @$.start, @$.end, 0); }
	;

cast_expr
	: unary_expr
	| '(' type_name ')' cast_expr
		{ push_node(AST_CAST, @$.start, @$.end, 2); }
	;

multiplicative_expr
	: cast_expr
	| multiplicative_expr '*' cast_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| multiplicative_expr '/' cast_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| multiplicative_expr '%' cast_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

additive_expr
	: multiplicative_expr
	| additive_expr '+' multiplicative_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| additive_expr '-' multiplicative_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

shift_expr
	: additive_expr
	| shift_expr LEFT_OP additive_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| shift_expr RIGHT_OP additive_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

relational_expr
	: shift_expr
	| relational_expr '<' shift_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| relational_expr '>' shift_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| relational_expr LE_OP shift_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| relational_expr GE_OP shift_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

equality_expr
	: relational_expr
	| equality_expr EQ_OP relational_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	| equality_expr NE_OP relational_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

and_expr
	: equality_expr
	| and_expr '&' equality_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

exclusive_or_expr
	: and_expr
	| exclusive_or_expr '^' and_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

inclusive_or_expr
	: exclusive_or_expr
	| inclusive_or_expr '|' exclusive_or_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

logical_and_expr
	: inclusive_or_expr
	| logical_and_expr AND_OP inclusive_or_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

logical_or_expr
	: logical_and_expr
	| logical_or_expr OR_OP logical_and_expr
		{ push_node(AST_BINARY, @$.start, @$.end, 2); }
	;

conditional_expr
	: logical_or_expr
	| logical_or_expr '?' logical_or_expr ':' conditional_expr
		{ push_node(AST_CONDITIONAL, @$.start, @$.end, 3); }
	;

assignment_expr
	: conditional_expr
	| unary_expr assignment_operator assignment_expr
		{ push_node(AST_ASSIGN, @$.start, @$.end, 2); }
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expr
	: assignment_expr
		{ push_node(AST_EXPRLIST, @$.start, @$.end, 1); }
	| expr ',' assignment_expr
		{ push_node(AST_EXPRLIST, @$.start, @$.end, 2); }
	;

constant_expr
	: conditional_expr
	;

declaration
	: declaration_specifiers ';'
		{ push_node(AST_DECLSTMT, @$.start, @$.end, 1); }
	| declaration_specifiers init_declarator_list ';'
		{ push_node(AST_DECLSTMT, @$.start, @$.end, 2); }
	;

declaration_specifiers
	: storage_class_specifier
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 1); }
	| storage_class_specifier declaration_specifiers
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 2); }
	| type_specifier
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 1); }
	| type_specifier declaration_specifiers
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 2); }
	| type_qualifier
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 1); }
	| type_qualifier declaration_specifiers
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 2); }
	| function_specifier
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 1); }
	| function_specifier declaration_specifiers
		{ push_node(AST_DECLSPEC, @$.start, @$.end, 2); }
	;

init_declarator_list
	: init_declarator
		{ push_node(AST_INITLIST, @$.start, @$.end, 1); }
	| init_declarator_list ',' init_declarator
		{ push_node(AST_INITLIST, @$.start, @$.end, 2); }
	;

init_declarator
	: declarator
	| declarator '=' initializer
		{ push_node(AST_INIT, @$.start, @$.end, 2); }
	;

storage_class_specifier
	: TYPEDEF
		{ push_node(AST_TYPEDEFKW, @$.start, @$.end, 0); }
	| EXTERN
		{ push_node(AST_EXTERNKW, @$.start, @$.end, 0); }
	| STATIC
		{ push_node(AST_STATICKW, @$.start, @$.end, 0); }
	| AUTO
		{ push_node(AST_AUTOKW, @$.start, @$.end, 0); }
	| REGISTER
		{ push_node(AST_REGISTERKW, @$.start, @$.end, 0); }
	;

type_specifier
	: VOID
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| CHAR
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| SHORT
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| INT
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| LONG
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| FLOAT
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| DOUBLE
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| SIGNED
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| UNSIGNED
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| BOOL
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| COMPLEX
		{ push_node(AST_TYPE, @$.start, @$.end, 0); }
	| struct_or_union_specifier
		{ push_node(AST_TYPE, @$.start, @$.end, 1); }
	| enum_specifier
		{ push_node(AST_TYPE, @$.start, @$.end, 1); }
	| TYPE_NAME
		{ push_node_name(AST_TYPENAME, @$.start, @$.end, $1); }
	;

struct_or_union_specifier
	: struct_or_union identifier '{' struct_declaration_list '}'
		{ push_node(AST_STRUCT, @$.start, @$.end, 3); }
	| struct_or_union '{' struct_declaration_list '}'
		{ push_node(AST_STRUCT, @$.start, @$.end, 2); }
	| struct_or_union identifier
		{ push_node(AST_STRUCT, @$.start, @$.end, 2); }
	;

struct_or_union
	: STRUCT
		{ push_node(AST_STRUCTKW, @$.start, @$.end, 0); }
	| UNION
		{ push_node(AST_UNIONKW, @$.start, @$.end, 0); }
	;

struct_declaration_list
	: struct_declaration
		{ push_node(AST_STRUCTLIST, @$.start, @$.end, 1); }
	| struct_declaration_list struct_declaration
		{ push_node(AST_STRUCTLIST, @$.start, @$.end, 2); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
		{ push_node(AST_STRUCTDECL, @$.start, @$.end, 2); }
	;

specifier_qualifier_list
	: type_specifier
		{ push_node(AST_STRUCTQUALLIST, @$.start, @$.end, 1); }
	| type_specifier specifier_qualifier_list
		{ push_node(AST_STRUCTQUALLIST, @$.start, @$.end, 2); }
	| type_qualifier
		{ push_node(AST_STRUCTQUALLIST, @$.start, @$.end, 1); }
	| type_qualifier specifier_qualifier_list
		{ push_node(AST_STRUCTQUALLIST, @$.start, @$.end, 2); }
	;

struct_declarator_list
	: struct_declarator
		{ push_node(AST_STRUCTDECLLIST, @$.start, @$.end, 1); }
	| struct_declarator_list ',' struct_declarator
		{ push_node(AST_STRUCTDECLLIST, @$.start, @$.end, 2); }
	;

struct_declarator
	: declarator
	| ':' constant_expr
		{ push_node(AST_STRUCTBITS, @$.start, @$.end, 2); }
	| declarator ':' constant_expr
		{ push_node(AST_STRUCTBITS, @$.start, @$.end, 2); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
		{ push_node(AST_ENUM, @$.start, @$.end, 1); }
	| ENUM identifier '{' enumerator_list '}'
		{ push_node(AST_ENUM, @$.start, @$.end, 2); }
	| ENUM '{' enumerator_list ',' '}'
		{ push_node(AST_ENUM, @$.start, @$.end, 1); }
	| ENUM identifier '{' enumerator_list ',' '}'
		{ push_node(AST_ENUM, @$.start, @$.end, 2); }
	| ENUM identifier
		{ push_node(AST_ENUM, @$.start, @$.end, 1); }
	;

enumerator_list
	: enumerator
		{ push_node(AST_ENUMLIST, @$.start, @$.end, 1); }
	| enumerator_list ',' enumerator
		{ push_node(AST_ENUMLIST, @$.start, @$.end, 2); }
	;

enumerator
	: identifier
		{ push_node(AST_ENUMVAL, @$.start, @$.end, 1); }
	| identifier '=' constant_expr
		{ push_node(AST_ENUMVAL, @$.start, @$.end, 2); }
	;

type_qualifier
	: CONST
		{ push_node(AST_TYPEQUAL, @$.start, @$.end, 0); }
	| RESTRICT
		{ push_node(AST_TYPEQUAL, @$.start, @$.end, 0); }
	| VOLATILE
		{ push_node(AST_TYPEQUAL, @$.start, @$.end, 0); }
	;

function_specifier
	: INLINE
		{ push_node(AST_FUNCSPEC, @$.start, @$.end, 0); }
	;

declarator
	: direct_declarator
		{ push_node(AST_DECL, @$.start, @$.end, 1); }
	| pointer direct_declarator
		{ push_node(AST_DECL, @$.start, @$.end, 2); }
	;

direct_declarator
	: identifier
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 1, DECL_ID); }
	| '(' declarator ')'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 1, DECL_INPARENS); }

	| direct_declarator '[' ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 1, DECL_BRACS); }
	| direct_declarator '[' type_qualifier_list ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_BRACS_TYPE); }
	| direct_declarator '[' assignment_expr ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_BRACS_ASSIGN); }
	| direct_declarator '[' type_qualifier_list assignment_expr ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 3, DECL_BRACS_BOTH); }

	| direct_declarator '[' STATIC assignment_expr ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_BRACS_STATIC_ASSIGN); }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expr ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 3, DECL_BRACS_STATIC_BOTH); }

	| direct_declarator '[' '*' ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 1, DECL_BRACS_STAR); }
	| direct_declarator '[' type_qualifier_list '*' ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_BRACS_TYPE_STAR); }

	| direct_declarator '[' type_qualifier_list STATIC assignment_expr ']'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 3, DECL_BRACS_BOTH_STATIC); }

	| direct_declarator '(' ')'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 1, DECL_PARENS); }
	| direct_declarator '(' parameter_type_list ')'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_PARENS_TYPE); }
	| direct_declarator '(' identifier_list ')'
		{ push_decl(AST_DIRDECL, @$.start, @$.end, 2, DECL_PARENS_ID); }
	;

pointer
	: '*'
		{ push_node(AST_PTR, @$.start, @$.end, 0); }
	| '*' type_qualifier_list
		{ push_node(AST_PTR, @$.start, @$.end, 1); }
	| '*' pointer
		{ push_node(AST_PTR, @$.start, @$.end, 1); }
	| '*' type_qualifier_list pointer
		{ push_node(AST_PTR, @$.start, @$.end, 2); }
	;

type_qualifier_list
	: type_qualifier
		{ push_node(AST_TYPEQUALLIST, @$.start, @$.end, 1); }
	| type_qualifier_list type_qualifier
		{ push_node(AST_TYPEQUALLIST, @$.start, @$.end, 2); }
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELIPSIS
	;

parameter_list
	: parameter_declaration
		{ push_node(AST_PARAMLIST, @$.start, @$.end, 1); }
	| parameter_list ',' parameter_declaration
		{ push_node(AST_PARAMLIST, @$.start, @$.end, 2); }
	;

parameter_declaration
	: declaration_specifiers
		{ push_node(AST_PARAMDECL, @$.start, @$.end, 1); }
	| declaration_specifiers declarator
		{ push_node(AST_PARAMDECL, @$.start, @$.end, 2); }
	| declaration_specifiers abstract_declarator
		{ push_node(AST_PARAMDECL, @$.start, @$.end, 2); }
	;

identifier_list
	: identifier
		{ push_node(AST_IDLIST, @$.start, @$.end, 1); }
	| identifier_list ',' identifier
		{ push_node(AST_IDLIST, @$.start, @$.end, 2); }
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
		{ push_node(AST_ATYPE, @$.start, @$.end, 2); }
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
		{ push_node(AST_ADECL, @$.start, @$.end, 2); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }

	| '[' ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 0); }
	| '[' assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }
	| '[' type_qualifier_list ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }
	| '[' type_qualifier_list assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 2); }
	| direct_abstract_declarator '[' ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }
	| direct_abstract_declarator '[' assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 2); }
	| direct_abstract_declarator '[' type_qualifier_list ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 2); }
	| direct_abstract_declarator '[' type_qualifier_list assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 3); }

	| '[' type_qualifier_list STATIC assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 2); }
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expr ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 3); }

	| '[' '*' ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 0); }
	| direct_abstract_declarator '[' '*' ']'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }

	| '(' ')'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 0); }
	| '(' parameter_type_list ')'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }
	| direct_abstract_declarator '(' ')'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 1); }
	| direct_abstract_declarator '(' parameter_type_list ')'
		{ push_node(AST_ADIRDECL, @$.start, @$.end, 2); }
	;

initializer
	: assignment_expr
	| '{' initializer_list '}'
		{ push_node(AST_INITIALIZER, @$.start, @$.end, 1); }
	| '{' initializer_list ',' '}'
		{ push_node(AST_INITIALIZER, @$.start, @$.end, 1); }
	;

initializer_list
	: initializer
		{ push_node(AST_INITIALIZERLIST, @$.start, @$.end, 1); }
	| designation initializer
		{ push_node(AST_INITIALIZERLIST, @$.start, @$.end, 2); }

	| initializer_list ',' initializer
		{ push_node(AST_INITIALIZERLIST, @$.start, @$.end, 2); }
	| initializer_list ',' designation initializer
		{ push_node(AST_INITIALIZERLIST, @$.start, @$.end, 3); }
	;

designation
	: designator_list '='
		{ push_node(AST_DESIGNATION, @$.start, @$.end, 1); }
	;

designator_list
	: designator
		{ push_node(AST_DESIGLIST, @$.start, @$.end, 1); }
	| designator_list designator
		{ push_node(AST_DESIGLIST, @$.start, @$.end, 2); }
	;

designator
	: '[' constant_expr ']'
		{ push_node(AST_DESIGNATOR, @$.start, @$.end, 1); }
	| '.' identifier
		{ push_node(AST_DESIGNATOR, @$.start, @$.end, 1); }

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	| macro
	;

labeled_statement
	: identifier ':' statement
		{ push_node(AST_LABELED, @$.start, @$.end, 2); }
	| CASE constant_expr ':' statement
		{ push_node(AST_CASED, @$.start, @$.end, 2); }
	| DEFAULT ':' statement
		{ push_node(AST_DEFAULTED, @$.start, @$.end, 1); }
	;

compound_statement
	: '{' '}'
		{ push_node(AST_BLOCK, @$.start, @$.end, 0); }
	| '{' block_item_list '}'
		{ push_node(AST_BLOCK, @$.start, @$.end, 1); }
	;

block_item_list
	: block_item
		{ push_node(AST_BLOCKLIST, @$.start, @$.end, 1); }
	| block_item_list block_item
		{ push_node(AST_BLOCKLIST, @$.start, @$.end, 2); }
	;

block_item
	: declaration
	| statement
	;

expression_statement
	: ';'
		{ push_node(AST_STMT, @$.start, @$.end, 0); }
	| expr ';'
		{ push_node(AST_STMT, @$.start, @$.end, 1); }
	;

selection_statement
	: IF '(' expr ')' statement
		{ push_node(AST_IF, @$.start, @$.end, 2); }
	| IF '(' expr ')' statement ELSE statement
		{ push_node(AST_IF, @$.start, @$.end, 3); }
	| SWITCH '(' expr ')' statement
		{ push_node(AST_SWITCH, @$.start, @$.end, 2); }
	;

iteration_statement
	: WHILE '(' expr ')' statement
		{ push_node(AST_WHILE, @$.start, @$.end, 2); }
	| DO statement WHILE '(' expr ')' ';'
		{ push_node(AST_DO, @$.start, @$.end, 2); }
	| FOR '(' ';' ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 1); }
	| FOR '(' ';' ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 2); }
	| FOR '(' ';' expr ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 2); }
	| FOR '(' ';' expr ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 3); }
	| FOR '(' expr ';' ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 2); }
	| FOR '(' expr ';' ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 3); }
	| FOR '(' expr ';' expr ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 3); }
	| FOR '(' expr ';' expr ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 4); }

	| FOR '(' declaration ';' ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 2); }
	| FOR '(' declaration ';' ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 3); }
	| FOR '(' declaration ';' expr ';' ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 3); }
	| FOR '(' declaration ';' expr ';' expr ')' statement
		{ push_node(AST_FOR, @$.start, @$.end, 4); }
	;

jump_statement
	: GOTO identifier ';'
		{ push_node(AST_GOTO, @$.start, @$.end, 1); }
	| CONTINUE ';'
		{ push_node(AST_CONTINUE, @$.start, @$.end, 0); }
	| BREAK ';'
		{ push_node(AST_BREAK, @$.start, @$.end, 0); }
	| RETURN ';'
		{ push_node(AST_RETURN, @$.start, @$.end, 0); }
	| RETURN expr ';'
		{ push_node(AST_RETURN, @$.start, @$.end, 1); }
	;

translation_unit
	: external_declaration
		{ push_node(AST_FILE, @$.start, @$.end, 1); }
	| translation_unit external_declaration
		{ push_node(AST_FILE, @$.start, @$.end, 2); }
	;

external_declaration
	: function_definition
	| declaration
	| macro
	;

function_definition
	: declaration_specifiers declarator compound_statement
		{ push_node(AST_FUNCTION, @$.start, @$.end, 3); }
	| declaration_specifiers declarator declaration_list compound_statement
		{ push_node(AST_FUNCTION, @$.start, @$.end, 4); }
	;

declaration_list
	: declaration
		{ push_node(AST_DECLLIST, @$.start, @$.end, 1); }
	| declaration_list declaration
		{ push_node(AST_DECLLIST, @$.start, @$.end, 2); }
	;

macro
	: MACRO
		{ push_node(AST_MACRO, @$.start, @$.end, 0); }
	;

identifier
	: IDENTIFIER
		{ push_node_name(AST_IDENTIFIER, @$.start, @$.end, $1); }
	;
%%

void yyerror(char *s)
{
	extern long token_offset;
	fprintf(stderr, "parsing error on %ld: %s\n", token_offset, s);
}
