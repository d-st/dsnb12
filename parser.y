%{

/*
 * parser.y -- SPL parser specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "scanner.h"

#include "sym.h"
#include "absyn.h"
#include "parser.h"

#define YYDEBUG 1

Absyn *progTree;

%}

%union {
  NoVal noVal;
  IntVal intVal;
  StringVal stringVal;
  Absyn *node;
}

%token<noVal>		ARRAY
%token<noVal>		ELSE
%token<noVal>		IF
%token<noVal>		OF
%token<noVal>		PROC
%token<noVal>		REF
%token<noVal>		TYPE
%token<noVal>		VAR
%token<noVal>		WHILE
%token<noVal>		LPAREN
%token<noVal>		RPAREN
%token<noVal>		LBRACK
%token<noVal>		RBRACK
%token<noVal>		LCURL
%token<noVal>		RCURL
%token<noVal>		EQ
%token<noVal>		NE
%token<noVal>		LT
%token<noVal>		LE
%token<noVal>		GT
%token<noVal>		GE
%token<noVal>		ASGN
%token<noVal>		COLON
%token<noVal>		COMMA
%token<noVal>		SEMIC
%token<noVal>		PLUS
%token<noVal>		MINUS
%token<noVal>		STAR
%token<noVal>		SLASH
%token<stringVal>	IDENT
%token<intVal>		INTLIT

%type<node>	program
%type<node>	typedef
%type<node>	procdef
%type<node>	parameter
%type<node>	parameters
%type<node>	parameters_list
%type<node>	lhs
%type<node>	expr_inner
%type<node>	expr_punkt
%type<node>	expr_strich
%type<node>	expr
%type<node>	arglist
%type<node>	statement
%type<node>	statements
%type<node>	declaration
%type<node>	declarations
%type<node>	type_t

%type<node>	start
%start		start

%%

parameter		: IDENT COLON type_t
			{ $$ = newParDec($1.line, newSym($1.val), $3, FALSE); }
			| REF IDENT COLON type_t
			{ $$ = newParDec($2.line, newSym($2.val), $4, TRUE); }
			;

parameters_list		: parameter COMMA parameters_list
			{ $$ = newDecList($1, $3); }
			| parameter
			{ $$ = newDecList($1, emptyDecList()); }
			;

parameters		: // nichts
			{ $$ = emptyDecList(); }
			| parameters_list
			{ $$ = $1; }
			;

lhs			: IDENT
			{ $$ = newSimpleVar($1.line, newSym($1.val)); }
			| IDENT LBRACK expr RBRACK
			{ $$ = newArrayVar($1.line, newSimpleVar($1.line, newSym($1.val)), $3); }
			;

expr_inner		: LPAREN expr RPAREN
			{ $$ = $2; }
			| INTLIT
			{ $$ = newIntExp($1.line, $1.val); }
			| IDENT
			{ $$ = newVarExp($1.line, newSimpleVar($1.line, newSym($1.val))); }
			| IDENT LBRACK expr RBRACK
			{ $$ = newVarExp($1.line, newArrayVar($1.line, newSimpleVar($1.line, newSym($1.val)), $3)); }
			| MINUS expr_inner
			{ $$ = newOpExp($1.line, ABSYN_OP_SUB, newIntExp($1.line, 0), $2); }
			;

expr_punkt		: expr_inner
			{ $$ = $1; }
			| expr_punkt STAR expr_inner
			{ $$ = newOpExp($2.line, ABSYN_OP_MUL, $1, $3); }
			| expr_punkt SLASH expr_inner
			{ $$ = newOpExp($2.line, ABSYN_OP_DIV, $1, $3); }
			;

expr_strich		: expr_punkt
			{ $$ = $1; }
			| expr_strich PLUS expr_punkt
			{ $$ = newOpExp($2.line, ABSYN_OP_ADD, $1, $3); }
			| expr_strich MINUS expr_punkt
			{ $$ = newOpExp($2.line, ABSYN_OP_SUB, $1, $3); }
			;

expr			: expr_strich
			{ $$ = $1; }
			| expr_strich LT expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_LST, $1, $3); }
			| expr_strich LE expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_LSE, $1, $3); }
			| expr_strich GT expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_GRT, $1, $3); }
			| expr_strich GE expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_GRE, $1, $3); }
			| expr_strich EQ expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_EQU, $1, $3); }
			| expr_strich NE expr_strich
			{ $$ = newOpExp($2.line, ABSYN_OP_NEQ, $1, $3); }
			;

arglist			: expr COMMA arglist
			{ $$ = newExpList($1, $3); }
			| expr
			{ $$ = newExpList($1, emptyExpList()); }
			;

statement		: SEMIC
			{ $$ = newEmptyStm($1.line); }
			| LCURL statements RCURL
			{ $$ = newCompStm($1.line, $2); }
			| lhs ASGN expr SEMIC
			{ $$ = newAssignStm($2.line, $1, $3); }
			| IF LPAREN expr RPAREN statement
			{ $$ = newIfStm($1.line, $3, $5, newEmptyStm($1.line)); }
			| IF LPAREN expr RPAREN statement ELSE statement
			{ $$ = newIfStm($1.line, $3, $5, $7); }
			| WHILE LPAREN expr RPAREN statement
			{ $$ = newWhileStm($1.line, $3, $5); }
			| IDENT LPAREN arglist RPAREN SEMIC
			{ $$ = newCallStm($1.line, newSym($1.val), $3); }
			;

statements		: statement statements
			{ $$ = newStmList($1, $2); }
			| // nichts
			{ $$ = emptyStmList(); }
			;

declaration		: VAR IDENT COLON type_t SEMIC
			{ $$ = newVarDec($1.line, newSym($2.val), $4); }
			;

declarations		: // nichts
			{ $$ = emptyDecList(); }
			| declaration declarations
			{ $$ = newDecList($1, $2); }
			;

procdef			: PROC IDENT LPAREN parameters RPAREN LCURL declarations statements RCURL
			{ $$ = newProcDec($1.line, newSym($2.val), $4, $7, $8) }
			;

type_t			: IDENT
			{ $$ = newNameTy($1.line, newSym($1.val)); }
			| ARRAY LBRACK INTLIT RBRACK OF type_t
			{ $$ = newArrayTy($1.line, $3.val, $6); }
			;

typedef			: TYPE IDENT EQ type_t SEMIC
			{ $$ = newTypeDec($1.line, newSym($2.val), $4); }
			;

program			: // nichts
			{ $$ = emptyDecList(); }
			| procdef program
			{ $$ = newDecList($1, $2); }
			| typedef program
			{ $$ = newDecList($1, $2); }
			;

start			: program
			{ progTree = $$ }
			;

%%


void yyerror(char *msg) {
  error("%s in line %d", msg, yylval.noVal.line);
}

