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

#define PREDEF_PROC (-1)

static int local_offset;

int compute_localvar_size(Bintree *bintree) {
  int size;
  if (bintree == NULL) {
    return 0;
  }
  if(bintree->entry->kind != ENTRY_KIND_VAR) {
    // SPL does not support nested definitions, so should never occur
    error("internal compiler error: nested definitions");
  }

  size = bintree->entry->u.varEntry.type->size;
  bintree->entry->u.varEntry.offset = local_offset;
  local_offset += size;

  size += compute_localvar_size(bintree->left);
  size += compute_localvar_size(bintree->right);
  return size;
}

static void compute_proc_arg_offsets(Entry *proc) {
  ParamTypes *params;
  int size = 0;
  if(proc->kind != ENTRY_KIND_PROC) 
    error("internal compiler error: compute_proc_size() evoked for non-procedure");

  // compute and save size of incoming args
  params = proc->u.procEntry.paramTypes;
  while(!params->isEmpty) {
    params->offset = size;
    size += params->type->size;
    params = params->next;
  }
  proc->u.procEntry.size_args_in = size;
}

static int compute_proc_localvars(Table *t, Absyn *aref) {
  if(aref == NULL) return -1; // build in proc
  Absyn *a = aref->u.procDec.decls;
  Absyn *b;
  Entry *e;
  int size, sum_size = 0;

  while(a && !a->u.decList.isEmpty) {
    b = a->u.decList.head;
    if(b->type == ABSYN_VARDEC) {
      e = lookup(t, b->u.varDec.name);
showTable(t);
printf("%s %p\n", symToString(b->u.varDec.name), e);//->u.varEntry.type);
      size = e->u.varEntry.type->size;
      sum_size += size;
      e->u.varEntry.offset = sum_size;
    }
    a = a->u.stmList.tail;
  }
  return sum_size;
}

static int compute_proc_out_sizes(Table *t, boolean *is_leaf, Absyn *aref) {
  int max_size = 0;
  int size;
  if(aref == NULL) return -1; // build in proc
  Absyn *a = aref->u.procDec.body;
  Absyn *b;

  *is_leaf  = TRUE;
  while(a && !a->u.stmList.isEmpty) {
    b = a->u.stmList.head;
    if(b->type == ABSYN_CALLSTM) {
      *is_leaf = FALSE;
      size = lookup(t, b->u.callStm.name)->u.procEntry.size_args_in;
      max_size = size>max_size?size:max_size;
    }
    a = a->u.stmList.tail;
  }
  return max_size;
}

static void compute_procs(Table *t, Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  if(bintree->entry->kind == ENTRY_KIND_PROC) {
    // compute arguments, parameters 
    compute_proc_arg_offsets(bintree->entry);
    // compute local vars
    compute_proc_localvars(t, bintree->entry->aref);
    // compute outgoing sizes
    bintree->entry->u.procEntry.size_args_out = compute_proc_out_sizes(t, &bintree->entry->u.procEntry.is_leaf, bintree->entry->aref);
  }
  compute_procs(t, bintree->left);
  compute_procs(t, bintree->right);
}

static void show_args(ParamTypes *paramTypes) {
  int n = 0;
  ParamTypes *p = paramTypes;
  while(!p->isEmpty) {
    printf("arg %d: sp + %d\n", ++n, p->offset);
    p = p->next;
  }
}

static void show_vars(Absyn *aref) {
/*
  if(aref->u.procDec.decls);
  printf("var '%s': fp - %d\n", symToString(bintree->sym), bintree->entry->u.varEntry.offset);
TODO
*/
}

static void show_VarAlloc(Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  if(bintree->entry->kind == ENTRY_KIND_PROC
      && bintree->entry->u.procEntry.size_local_vars != PREDEF_PROC) {
    printf("\nVariable allocation for procedure '%s'\n", symToString(bintree->sym));
    show_args(bintree->entry->u.procEntry.paramTypes);
    printf("size of argument area = %d\n", bintree->entry->u.procEntry.size_args_in);
    show_vars(bintree->entry->aref);
    printf("size of localvar area = %d\n", bintree->entry->u.procEntry.size_local_vars);
    printf("size of outgoing area = %d\n", bintree->entry->u.procEntry.size_args_out);
  }
  show_VarAlloc(bintree->left);
  show_VarAlloc(bintree->right);
}

void allocVars(Absyn *program, Table *globalTable, boolean showVarAlloc) {
  /* compute access information for arguments of predefined procs */
  Entry *e;
  e = lookup(globalTable, newSym("exit"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("time"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("readi"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("readc"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("printi"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("printc"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("clearAll"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("setPixel"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("drawCircle"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  e = lookup(globalTable, newSym("drawLine"));
  e->u.procEntry.size_args_out   = 0;
  e->u.procEntry.size_local_vars = PREDEF_PROC;

  /* compute access information for all procedures in the global table */
  compute_procs(globalTable, globalTable->bintree);
  /* run again, so outgoing area sizes can be filled from initialized values */
  compute_procs(globalTable, globalTable->bintree);

  /* show variable allocation if requested */
  if(showVarAlloc) {
    show_VarAlloc(globalTable->bintree);
  }
}

