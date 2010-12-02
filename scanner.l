%{

/*
 * scanner.l -- SPL scanner specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "scanner.h"
#include "sym.h"
#include "absyn.h"
#include "parser.tab.h"

static int lineNumber = 1;

%}


%%

"array"				{ yylval.noVal.line = lineNumber; return ARRAY; }
"else"				{ yylval.noVal.line = lineNumber; return ELSE; }
"if"				{ yylval.noVal.line = lineNumber; return IF; }
"of"				{ yylval.noVal.line = lineNumber; return OF; }
"proc"				{ yylval.noVal.line = lineNumber; return PROC; }
"ref"				{ yylval.noVal.line = lineNumber; return REF; }
"type"				{ yylval.noVal.line = lineNumber; return TYPE; }
"var"				{ yylval.noVal.line = lineNumber; return VAR; }
"while"				{ yylval.noVal.line = lineNumber; return WHILE; }
"("				{ yylval.noVal.line = lineNumber; return LPAREN; }
")"				{ yylval.noVal.line = lineNumber; return RPAREN; }
"["				{ yylval.noVal.line = lineNumber; return LBRACK; }
"]"				{ yylval.noVal.line = lineNumber; return RBRACK; }
"{"				{ yylval.noVal.line = lineNumber; return LCURL; }
"}"				{ yylval.noVal.line = lineNumber; return RCURL; }
"="				{ yylval.noVal.line = lineNumber; return EQ; }
"#"				{ yylval.noVal.line = lineNumber; return NE; }
"<"				{ yylval.noVal.line = lineNumber; return LT; }
"<="				{ yylval.noVal.line = lineNumber; return LE; }
">"				{ yylval.noVal.line = lineNumber; return GT; }
">="				{ yylval.noVal.line = lineNumber; return GE; }
":="				{ yylval.noVal.line = lineNumber; return ASGN; }
":"				{ yylval.noVal.line = lineNumber; return COLON; }
","				{ yylval.noVal.line = lineNumber; return COMMA; }
";"				{ yylval.noVal.line = lineNumber; return SEMIC; }
"+"				{ yylval.noVal.line = lineNumber; return PLUS; }
"-"				{ yylval.noVal.line = lineNumber; return MINUS; }
"*"				{ yylval.noVal.line = lineNumber; return STAR; }
"/"				{ yylval.noVal.line = lineNumber; return SLASH; }

[_a-zA-Z][_a-zA-Z0-9]*		{ yylval.noVal.line = lineNumber; 
                                  yylval.stringVal.val = (char *)strdup(yytext); return IDENT; }
[0-9]+				{ yylval.noVal.line = lineNumber; 
                                  yylval.intVal.val = atoi(yytext); return INTLIT; }
0x[0-9a-fA-F]+			{ yylval.intVal.line = lineNumber; 
                                  yylval.intVal.val = strtol(yytext, NULL, 16); return INTLIT; }
'.'				{ yylval.intVal.line = lineNumber; 
                                  yylval.intVal.val = *(yytext+1); return INTLIT; }
'\\n'				{ yylval.intVal.line = lineNumber; 
                                  yylval.intVal.val = '\n'; return INTLIT; }

\/{2}.*				{ /* Zeilenkommentar */ }
[ \t\r]+			{ /* Whitespace, kein Token */ }
\n				{ lineNumber++;  /* Zeilenumbruch */}
<<EOF>>				{ yylval.noVal.line = lineNumber; return EOF; }
.				{ error("nicht erkanntes Zeichen \"%s\" in Zeile %d\n", yytext, lineNumber); }

%%


int yywrap(void) {
  return 1;
}


void showToken(int token) {
  switch(token) {
    case ARRAY:  printf("TOKEN = ARRAY in line %d\n", yylval.noVal.line); break;
    case ELSE:   printf("TOKEN = ELSE in line %d\n", yylval.noVal.line); break;
    case IF:     printf("TOKEN = IF in line %d\n", yylval.noVal.line); break;
    case OF:     printf("TOKEN = OF in line %d\n", yylval.noVal.line); break; 
    case PROC:   printf("TOKEN = PROC in line %d\n", yylval.noVal.line); break;
    case REF:    printf("TOKEN = REF in line %d\n", yylval.noVal.line); break;
    case TYPE:   printf("TOKEN = TYPE in line %d\n", yylval.noVal.line); break;
    case VAR:    printf("TOKEN = VAR in line %d\n", yylval.noVal.line); break;
    case WHILE:  printf("TOKEN = WHILE in line %d\n", yylval.noVal.line); break;
    case LPAREN: printf("TOKEN = LPAREN in line %d\n", yylval.noVal.line); break;
    case RPAREN: printf("TOKEN = RPAREN in line %d\n", yylval.noVal.line); break; 
    case LBRACK: printf("TOKEN = LBRACK in line %d\n", yylval.noVal.line); break; 
    case RBRACK: printf("TOKEN = RBRACK in line %d\n", yylval.noVal.line); break; 
    case LCURL:  printf("TOKEN = LCURL in line %d\n", yylval.noVal.line); break;
    case RCURL:  printf("TOKEN = RCURL in line %d\n", yylval.noVal.line); break; 
    case EQ:     printf("TOKEN = EQ in line %d\n", yylval.noVal.line); break;
    case NE:     printf("TOKEN = NE in line %d\n", yylval.noVal.line); break; 
    case LT:     printf("TOKEN = LT in line %d\n", yylval.noVal.line); break; 
    case LE:     printf("TOKEN = LE in line %d\n", yylval.noVal.line); break; 
    case GT:     printf("TOKEN = GT in line %d\n", yylval.noVal.line); break; 
    case GE:     printf("TOKEN = GE in line %d\n", yylval.noVal.line); break; 
    case ASGN:   printf("TOKEN = ASGN in line %d\n", yylval.noVal.line); break;
    case COLON:  printf("TOKEN = COLON in line %d\n", yylval.noVal.line); break;
    case COMMA:  printf("TOKEN = COMMA in line %d\n", yylval.noVal.line); break; 
    case SEMIC:  printf("TOKEN = SEMIC in line %d\n", yylval.noVal.line); break; 
    case PLUS:   printf("TOKEN = PLUS in line %d\n", yylval.noVal.line); break; 
    case MINUS:  printf("TOKEN = MINUS in line %d\n", yylval.noVal.line); break; 
    case STAR:   printf("TOKEN = STAR in line %d\n", yylval.noVal.line); break; 
    case SLASH:  printf("TOKEN = SLASH in line %d\n", yylval.noVal.line); break; 
    case IDENT:  printf("TOKEN = IDENT in line %d, value = \"%s\"\n", yylval.stringVal.line, yylval.stringVal.val); break; 
    case INTLIT: printf("TOKEN = INTLIT in line %d, value = %d\n", yylval.intVal.line, yylval.intVal.val); break; 
    case EOF:    printf("TOKEN = -- EOF --\n"); exit(0); break; 
  }
}

