/*
 * codegen.c -- ECO32 code generator
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
#include "codegen.h"
#include "varalloc.h"

#define R_MIN   8  // lowest free register
#define R_MAX   23 // highest free register
#define R_RET   31 // return address
#define R_FP    25 // frame pointer
#define R_SP    29 // stack pointer

static FILE *file;
static int label_else = -1;

void genCodePostorder(Absyn *node, Table *symtab, int r);

int newLabel() {
  static unsigned n=0;
  return n++;
}

void assemblerProlog() {
  fprintf(file, "\t.import\tprinti\n");
  fprintf(file, "\t.import\tprintc\n");
  fprintf(file, "\t.import\treadi\n");
  fprintf(file, "\t.import\treadc\n");
  fprintf(file, "\t.import\texit\n");
  fprintf(file, "\t.import\ttime\n");
  fprintf(file, "\t.import\tclearAll\n");
  fprintf(file, "\t.import\tsetPixel\n");
  fprintf(file, "\t.import\tdrawLine\n");
  fprintf(file, "\t.import\tdrawCircle\n");
  fprintf(file, "\t.import\t_indexError\n");
  fprintf(file, "\n");
  fprintf(file, "\t.code\n");
  fprintf(file, "\t.align\t4\n");
}

int isParmRef(ParamTypes *paramTypes, int n) {
  int i = 0;
  while (i < n) {
    paramTypes = paramTypes->next;
    i++;
  }
  return paramTypes->isRef;
}

void do_args(Absyn *args, Table *symtab, Sym *sym, int n, int r) {
  if(args->u.expList.isEmpty) return;
  genCodePostorder(args->u.expList.head, symtab, r);

  if(!isParmRef(lookup(symtab, sym)->u.procEntry.paramTypes, n)) {
    fprintf(file, "\tldw\t$%d,$%d,0\n", r, r);
  }

  fprintf(file, "\tstw\t$%d,$%d,%d\t\t; store arg #%d\n", r, R_SP, n*INT_BYTE_SIZE, n);
  do_args(args->u.expList.tail, symtab, sym, n+1, r);
}

void genCodePostorderOpExp(Absyn *node, Table *symtab, int r) {
  int temp = r+1;
  if(temp>R_MAX) error("Ausdruck zu kompliziert\n");
  genCodePostorder(node->u.opExp.left, symtab, r);
  genCodePostorder(node->u.opExp.right, symtab, temp);
  switch(node->u.opExp.op) {
    case ABSYN_OP_ADD:
      fprintf(file, "\tadd");
      break;
    case ABSYN_OP_SUB:
      fprintf(file, "\tsub");
      break;
    case ABSYN_OP_MUL:
      fprintf(file, "\tmul");
      break;
    case ABSYN_OP_DIV:
      fprintf(file, "\tdiv");
      break;
    
    case ABSYN_OP_EQU:
      fprintf(file, "\tbne\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    case ABSYN_OP_NEQ:
      fprintf(file, "\tbeq\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    case ABSYN_OP_LST:
      fprintf(file, "\tbge\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    case ABSYN_OP_LSE:
      fprintf(file, "\tbgt\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    case ABSYN_OP_GRT:
      fprintf(file, "\tble\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    case ABSYN_OP_GRE:
      fprintf(file, "\tblt\t$%d,$%d,L%d\n", r, r+1, label_else);
      return; break;
    
    default:
      error("ICE: unhandled op %d\n", node->u.opExp.op);
  }
  fprintf(file, "\t$%d,$%d,$%d\n", r, r, temp);
}

void genCodePostorderCompStm(Absyn *node, Table *symtab, int r) {
  genCodePostorder(node->u.compStm.stms, symtab, r);
}

void genCodePostorderAssignStm(Absyn *node, Table *symtab, int r) {
  Absyn *var = node->u.assignStm.var;
  Absyn *exp = node->u.assignStm.exp;
  
  genCodePostorder(var, symtab, r);
  genCodePostorder(exp, symtab, r+1);
  fprintf(file, "\tstw\t$%d,$%d,0\n", r+1, r);
}

void genCodePostorderIfStm(Absyn *node, Table *symtab, int r) {
  int label_ende, old_else=label_else;
  boolean has_else = node->u.ifStm.elsePart->u.compStm.stms != NULL;
  
  label_else = newLabel();
  if(has_else) {
    label_ende = newLabel();
  }
  
  genCodePostorder(node->u.ifStm.test, symtab, r);
  genCodePostorder(node->u.ifStm.thenPart, symtab, r);

  if(has_else) {
    fprintf(file, "\tj\tL%d\n", label_ende);
  }

  fprintf(file, "L%d:\n", label_else);
  if(has_else) {
    genCodePostorder(node->u.ifStm.elsePart, symtab, r);
    fprintf(file, "L%d:\n", label_ende);
  }
  label_else=old_else;
}

void genCodePostorderWhileStm(Absyn *node, Table *symtab, int r) {
  int label = newLabel();
  label_else = newLabel();
  
  fprintf(file, "L%d:\n", label);
  genCodePostorder(node->u.whileStm.test, symtab, r);
  genCodePostorder(node->u.whileStm.body, symtab, r);
  fprintf(file, "\tj\tL%d\n", label);
  fprintf(file, "L%d:\n", label_else);
}

void genCodePostorderCallStm(Absyn *node, Table *symtab, int r) {
  do_args(node->u.callStm.args, symtab, node->u.callStm.name, 0, r);
  fprintf(file, "\tjal\t%s\n", symToString(node->u.callStm.name));
}

void genCodePostorderIntExp(Absyn *node, Table *symtab, int r) {
  fprintf(file, "\tadd\t$%d,$0,%d\n", r, node->u.intExp.val);
}

void genCodePostorderVarExp(Absyn *node, Table *symtab, int r) {
  genCodePostorder(node->u.varExp.var, symtab, r);
}

void genCodePostorderSimpleVar(Absyn *node, Table *symtab, int r) {
  Entry *e = lookup(symtab, node->u.simpleVar.name);

  if(e->source == ENTRY_SOURCE_PARAM) {
    fprintf(file, "\tadd\t$%d,$%d,%d\n", r, R_FP, (e->u.varEntry.offset));
    fprintf(file, "\tldw\t$%d,$%d,0\n", r, r);
  } else {
    fprintf(file, "\tadd\t$%d,$%d,%d\n", r, R_FP, -(e->u.varEntry.offset));
  }

}

void genCodePostorderArrayVar(Absyn *node, Table *symtab, int r) {
  int actualarraysize, basetypesize;
  if(r+2>R_MAX) error("zuwenig Register in genCodePostorderArrayVar()");

  genCodePostorder(node->u.arrayVar.var, symtab, r);
  genCodePostorder(node->u.arrayVar.index, symtab, r+1);

  basetypesize = node->type_t->size;
  actualarraysize = node->u.arrayVar.var->type_t->u.arrayType.size;

  fprintf(file, "\tadd\t$%d,$%d,%d\n", r+2, 0, actualarraysize);
  fprintf(file, "\tbgeu\t$%d,$%d,_indexError\n", r+1, r+2);
  fprintf(file, "\tmul\t$%d,$%d,%d\n", r+1, r+1, basetypesize);
  fprintf(file, "\tadd\t$%d,$%d,$%d\n", r, r, r+1);
}

void genCodePostorderDecList(Absyn *node, Table *symtab, int r) {
  if(!node->u.decList.isEmpty) {
    genCodePostorder(node->u.decList.head, symtab, r);
    genCodePostorder(node->u.decList.tail, symtab, r);
  }
}

void genCodePostorderStmList(Absyn *node, Table *symtab, int r) {
  if(!node->u.stmList.isEmpty) {
    genCodePostorder(node->u.stmList.head, symtab, r);
    genCodePostorder(node->u.stmList.tail, symtab, r);
  }
}

void genCodePostorderExpList(Absyn *node, Table *symtab, int r) {
  if(!node->u.expList.isEmpty) {
    genCodePostorder(node->u.expList.head, symtab, r);
    genCodePostorder(node->u.expList.tail, symtab, r);
  }
}

void genCodePostorderProcDec(Absyn *node, Table *symtab, int r) {
  char *name = symToString(node->u.procDec.name);
  Entry *e = lookup(symtab, node->u.procDec.name);
  int outsize = e->u.procEntry.size_args_out;
  int framesize, oldfp, oldret;
  int noleaf = (outsize == LEAF_PROC)?0:1;
  
  if(noleaf) {
    framesize = e->u.procEntry.size_local_vars
                + 2*REF_BYTE_SIZE + outsize;
    oldfp  = e->u.procEntry.size_local_vars + REF_BYTE_SIZE;
  } else {
    framesize = e->u.procEntry.size_local_vars + REF_BYTE_SIZE;
    oldfp  = 0;
  }
  oldret = e->u.procEntry.size_local_vars + 2*REF_BYTE_SIZE;
  
  fprintf(file, "\n\t.export\t%s\n", name);
  fprintf(file, "%s:\n", name);
  fprintf(file, "\tsub\t$%d,$%d,%d\t\t; allocate frame\n", R_SP, R_SP, framesize);
  fprintf(file, "\tstw\t$%d,$%d,%d\t\t; save old frame pointer\n", R_FP, R_SP, oldfp);
  fprintf(file, "\tadd\t$%d,$%d,%d\t\t; setup new frame pointer\n", R_FP, R_SP, framesize);

  if(noleaf)
  fprintf(file, "\tstw\t$%d,$%d,%d\t\t; save return register\n", R_RET, R_FP, -oldret);

  genCodePostorder(node->u.procDec.body, e->u.procEntry.localTable, r);

  if(noleaf)
  fprintf(file, "\tldw\t$%d,$%d,%d\t\t; restore return register\n", R_RET, R_FP, -oldret);
  
  
  fprintf(file, "\tldw\t$%d,$%d,%d\t\t; restore old frame pointer\n", R_FP, R_SP, oldfp);
  fprintf(file, "\tadd\t$%d,$%d,%d\t\t; release frame\n", R_SP, R_SP, framesize);
  fprintf(file, "\tjr\t$%d\t\t\t; return\n", R_RET);
}

void genCodePostorder(Absyn *node, Table *symtab, int r) {
	switch (node->type) {
	case ABSYN_NAMETY:
	case ABSYN_ARRAYTY:
	case ABSYN_TYPEDEC:
	case ABSYN_PARDEC:
	case ABSYN_EMPTYSTM:
	case ABSYN_VARDEC:
		// the above cases are not relevant for code generation
        return;	break;
    
	case ABSYN_PROCDEC:
		genCodePostorderProcDec(node, symtab, r);
        return;	break;
	case ABSYN_COMPSTM:
		genCodePostorderCompStm(node, symtab, r);
        return;	break;
	case ABSYN_ASSIGNSTM:
		genCodePostorderAssignStm(node, symtab, r);
        return;	break;
	case ABSYN_IFSTM:
		genCodePostorderIfStm(node, symtab, r);
        return;	break;
	case ABSYN_WHILESTM:
		genCodePostorderWhileStm(node, symtab, r);
        return;	break;
	case ABSYN_CALLSTM:
		genCodePostorderCallStm(node, symtab, r);
        return;	break;
	case ABSYN_OPEXP:
		genCodePostorderOpExp(node, symtab, r);
        return;	break;
	case ABSYN_VAREXP:
		genCodePostorderVarExp(node, symtab, r);
        return;	break;
	case ABSYN_INTEXP:
		genCodePostorderIntExp(node, symtab, r);
        return;	break;
	case ABSYN_SIMPLEVAR:
		genCodePostorderSimpleVar(node, symtab, r);
        return;	break;
	case ABSYN_ARRAYVAR:
		genCodePostorderArrayVar(node, symtab, r);
        return;	break;
	case ABSYN_DECLIST:
		genCodePostorderDecList(node, symtab, r);
        return;	break;
	case ABSYN_STMLIST:
		genCodePostorderStmList(node, symtab, r);
        return;	break;
	case ABSYN_EXPLIST:
		genCodePostorderExpList(node, symtab, r);
        return;	break;
	default:
		error("unknown node type %d in genCodePostorder, line %d", node->type,
		      node->line);
	}
}

void genCode(Absyn *program, Table *globalTable, FILE *outfile) {
  
  file = outfile;
//  file = stdout;
  
  assemblerProlog();
  genCodePostorder(program, globalTable, R_MIN);
}
