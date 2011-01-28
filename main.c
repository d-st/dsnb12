/*
 * main.c -- SPL compiler
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "absyn.h"
#include "scanner.h"
#include "parser.h"
#include "table.h"
#include "semant.h"
#include "varalloc.h"
#include "codegen.h"


#define VERSION		"1.1"


static void version(char *myself) {
  printf("%s version %s (compiled %s)\n", myself, VERSION, __DATE__);
}


static void help(char *myself) {
  /* show some help how to use the program */
  printf("Usage: %s [options] <input file> <output file>\n", myself);
  printf("Options:\n");
  printf("  --tokens         show stream of tokens\n");
  printf("  --absyn          show abstract syntax\n");
  printf("  --tables         show symbol tables\n");
  printf("  --vars           show variable allocation\n");
  printf("  --version        show compiler version\n");
  printf("  --help           show this help\n");
}


int main(int argc, char *argv[]) {
  int i;
  char *inFileName;
  char *outFileName;
  boolean optionTokens;
  boolean optionAbsyn;
  boolean optionTables;
  boolean optionVars;
  int token;
  Table *globalTable;
  FILE *outFile;

  /* analyze command line */
  inFileName = NULL;
  outFileName = NULL;
  optionTokens = FALSE;
  optionAbsyn = FALSE;
  optionTables = FALSE;
  optionVars = FALSE;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      /* option */
      if (strcmp(argv[i], "--tokens") == 0) {
        optionTokens = TRUE;
      } else
      if (strcmp(argv[i], "--absyn") == 0) {
        optionAbsyn = TRUE;
      } else
      if (strcmp(argv[i], "--tables") == 0) {
        optionTables = TRUE;
      } else
      if (strcmp(argv[i], "--vars") == 0) {
        optionVars = TRUE;
      } else
      if (strcmp(argv[i], "--version") == 0) {
        version(argv[0]);
        exit(0);
      } else
      if (strcmp(argv[i], "--help") == 0) {
        help(argv[0]);
        exit(0);
      } else {
        error("unrecognized option '%s'; try '%s --help'",
              argv[i], argv[0]);
      }
    } else {
      /* file */
      if (outFileName != NULL) {
        error("more than two file names not allowed");
      }
      if (inFileName != NULL) {
        outFileName = argv[i];
      } else {
        inFileName = argv[i];
      }
    }
  }
  if (inFileName == NULL) {
    error("no input file");
  }
  if (outFileName == NULL) {
    error("no output file");
  }
  yyin = fopen(inFileName, "r");
  if (yyin == NULL) {
    error("cannot open input file '%s'", inFileName);
  }
  if (optionTokens) {
    do {
      token = yylex();
      showToken(token);
    } while (token != 0);
    fclose(yyin);
    exit(0);
  }
  yyparse();
  fclose(yyin);
  if (optionAbsyn) {
    showAbsyn(progTree);
    exit(0);
  }
  globalTable = check(progTree, optionTables);
  allocVars(progTree, globalTable, optionVars);
  outFile = fopen(outFileName, "w");
  if (outFile == NULL) {
    error("cannot open output file '%s'", outFileName);
  }
  genCode(progTree, globalTable, outFile);
  fclose(outFile);
  return 0;
}
