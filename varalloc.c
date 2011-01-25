/*
 * varalloc.c -- variable allocation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "absyn.h"
#include "table.h"
#include "varalloc.h"


void allocVars(Absyn *program, Table *globalTable, boolean showVarAlloc) {
  /* compute access information for arguments of predefined procs */
  /* compute access information for arguments, parameters and local vars */
  /* compute outgoing area sizes */
  /* show variable allocation if requested */
}
