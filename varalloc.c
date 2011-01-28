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

static void compute_proc_arg_offsets(Entry *proc) {
  ParamTypes *params;
  int size = 0;
  if(proc->kind != ENTRY_KIND_PROC) 
    error("internal compiler error: compute_proc_size() evoked for non-procedure");

  // compute and save size of incoming args
  params = proc->u.procEntry.paramTypes;
  while(!params->isEmpty) {
    params->offset = size;
    size += params->isRef?REF_BYTE_SIZE:params->type->size;
    params = params->next;
  }
  proc->u.procEntry.size_args_in = size;
}

static int compute_proc_localvars(Table *t, Absyn *aref) {
  if(aref == NULL) return PREDEF_PROC; // build in proc
  Absyn *a = aref->u.procDec.decls;
  Absyn *b;
  Entry *e;
  int size, sum_size = 0;

  while(a && !a->u.decList.isEmpty) {
    b = a->u.decList.head;
    if(b->type == ABSYN_VARDEC) {
      e = lookup(t, b->u.varDec.name);
      size = e->u.varEntry.type->size;
      sum_size += size;
      e->u.varEntry.offset = sum_size;
    }
    a = a->u.stmList.tail;
  }
  return sum_size;
}

static void compute_proc_param_offsets(Bintree *bintree, boolean reset) {
  static int offset = 0;
  if(reset) offset = 0;
  if(bintree == NULL) return;
  if(bintree->entry->source == ENTRY_SOURCE_PARAM) {
    bintree->entry->u.varEntry.offset = offset;
    if(bintree->entry->u.varEntry.isRef)
      offset += REF_BYTE_SIZE;
    else
      offset += bintree->entry->u.varEntry.type->size;
  }
  compute_proc_param_offsets(bintree->left,  FALSE);
  compute_proc_param_offsets(bintree->right, FALSE);
}


static int compute_proc_out_sizes(Table *t, Absyn *a) {
  boolean is_leaf = TRUE;
  int max_size = 0;
  int size = LEAF_PROC, size2;
  if(a == NULL) return PREDEF_PROC; // build in proc

  switch(a->type) {
    case ABSYN_PROCDEC:
      size = compute_proc_out_sizes(t, a->u.procDec.body);
      break;
    case ABSYN_STMLIST:
      if(a->u.stmList.isEmpty) break;
      size = compute_proc_out_sizes(t, a->u.stmList.head);
      size2 = compute_proc_out_sizes(t, a->u.stmList.tail);
      if(size2>size) size=size2;
      break;
    case ABSYN_CALLSTM:
      size = lookup(t, a->u.callStm.name)->u.procEntry.size_args_in;
      break;
    case ABSYN_EMPTYSTM: break;
    case ABSYN_COMPSTM:
      size = compute_proc_out_sizes(t, a->u.compStm.stms);
      break;
    case ABSYN_WHILESTM: 
      size = compute_proc_out_sizes(t, a->u.whileStm.body);
      break;
    case ABSYN_IFSTM:
      size = compute_proc_out_sizes(t, a->u.ifStm.thenPart);
      size2 = compute_proc_out_sizes(t, a->u.ifStm.elsePart);
      if(size2>size) size=size2;
      break;
    default:
      break;
  }
  if(size != LEAF_PROC) is_leaf = FALSE;
  max_size = size>max_size?size:max_size;
  return is_leaf?LEAF_PROC:max_size;
}

static void compute_procs(Table *t, Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  if(bintree->entry->kind == ENTRY_KIND_PROC) {
    // compute arguments, parameters 
    compute_proc_arg_offsets(bintree->entry);
    compute_proc_param_offsets(bintree->entry->u.procEntry.localTable->bintree, TRUE);
    // compute local vars
    bintree->entry->u.procEntry.size_local_vars = compute_proc_localvars(bintree->entry->u.procEntry.localTable, bintree->entry->aref);
    // compute outgoing sizes
    bintree->entry->u.procEntry.size_args_out = compute_proc_out_sizes(t, bintree->entry->aref);
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

static void show_parms(Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  if(bintree->entry->kind == ENTRY_KIND_VAR
    && bintree->entry->source == ENTRY_SOURCE_PARAM) {
    printf("param '%s': fp + %d\n", symToString(bintree->sym), bintree->entry->u.varEntry.offset);
  }
  show_parms(bintree->left);
  show_parms(bintree->right);
}

static void show_vars(Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  if(bintree->entry->kind == ENTRY_KIND_VAR
    && bintree->entry->source == ENTRY_SOURCE_LOCVAR) {
    printf("var '%s': fp - %d\n", symToString(bintree->sym), bintree->entry->u.varEntry.offset);
  }
  show_vars(bintree->left);
  show_vars(bintree->right);
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
    show_parms(bintree->entry->u.procEntry.localTable->bintree);
    show_vars(bintree->entry->u.procEntry.localTable->bintree);
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

