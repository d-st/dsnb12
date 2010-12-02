/*
 * absyn.c -- abstract syntax
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"


/**************************************************************/


Absyn *newNameTy(int line, Sym *name) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NAMETY;
  node->line = line;
  node->u.nameTy.name = name;
  return node;
}


Absyn *newArrayTy(int line, int size, Absyn *ty) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ARRAYTY;
  node->line = line;
  node->u.arrayTy.size = size;
  node->u.arrayTy.ty = ty;
  return node;
}


Absyn *newTypeDec(int line, Sym *name, Absyn *ty) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_TYPEDEC;
  node->line = line;
  node->u.typeDec.name = name;
  node->u.typeDec.ty = ty;
  return node;
}


Absyn *newProcDec(int line, Sym *name,
                  Absyn *params, Absyn *decls, Absyn *body) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_PROCDEC;
  node->line = line;
  node->u.procDec.name = name;
  node->u.procDec.params = params;
  node->u.procDec.decls = decls;
  node->u.procDec.body = body;
  return node;
}


Absyn *newParDec(int line, Sym *name, Absyn *ty, boolean isRef) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_PARDEC;
  node->line = line;
  node->u.parDec.name = name;
  node->u.parDec.ty = ty;
  node->u.parDec.isRef = isRef;
  return node;
}


Absyn *newVarDec(int line, Sym *name, Absyn *ty) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VARDEC;
  node->line = line;
  node->u.varDec.name = name;
  node->u.varDec.ty = ty;
  return node;
}


Absyn *newEmptyStm(int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EMPTYSTM;
  node->line = line;
  return node;
}


Absyn *newCompStm(int line, Absyn *stms) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_COMPSTM;
  node->line = line;
  node->u.compStm.stms = stms;
  return node;
}


Absyn *newAssignStm(int line, Absyn *var, Absyn *exp) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ASSIGNSTM;
  node->line = line;
  node->u.assignStm.var = var;
  node->u.assignStm.exp = exp;
  return node;
}


Absyn *newIfStm(int line, Absyn *test, Absyn *thenPart, Absyn *elsePart) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_IFSTM;
  node->line = line;
  node->u.ifStm.test = test;
  node->u.ifStm.thenPart = thenPart;
  node->u.ifStm.elsePart = elsePart;
  return node;
}


Absyn *newWhileStm(int line, Absyn *test, Absyn *body) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_WHILESTM;
  node->line = line;
  node->u.whileStm.test = test;
  node->u.whileStm.body = body;
  return node;
}


Absyn *newCallStm(int line, Sym *name, Absyn *args) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CALLSTM;
  node->line = line;
  node->u.callStm.name = name;
  node->u.callStm.args = args;
  return node;
}


Absyn *newOpExp(int line, int op, Absyn *left, Absyn *right) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_OPEXP;
  node->line = line;
  node->u.opExp.op = op;
  node->u.opExp.left = left;
  node->u.opExp.right = right;
  return node;
}


Absyn *newVarExp(int line, Absyn *var) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VAREXP;
  node->line = line;
  node->u.varExp.var = var;
  return node;
}


Absyn *newIntExp(int line, int val) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_INTEXP;
  node->line = line;
  node->u.intExp.val = val;
  return node;
}


Absyn *newSimpleVar(int line, Sym *name) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_SIMPLEVAR;
  node->line = line;
  node->u.simpleVar.name = name;
  return node;
}


Absyn *newArrayVar(int line, Absyn *var, Absyn *index) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ARRAYVAR;
  node->line = line;
  node->u.arrayVar.var = var;
  node->u.arrayVar.index = index;
  return node;
}


Absyn *emptyDecList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_DECLIST;
  node->line = -1;
  node->u.decList.isEmpty = TRUE;
  return node;
}


Absyn *newDecList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_DECLIST;
  node->line = -1;
  node->u.decList.isEmpty = FALSE;
  node->u.decList.head = head;
  node->u.decList.tail = tail;
  return node;
}


Absyn *emptyStmList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STMLIST;
  node->line = -1;
  node->u.stmList.isEmpty = TRUE;
  return node;
}


Absyn *newStmList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STMLIST;
  node->line = -1;
  node->u.stmList.isEmpty = FALSE;
  node->u.stmList.head = head;
  node->u.stmList.tail = tail;
  return node;
}


Absyn *emptyExpList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EXPLIST;
  node->line = -1;
  node->u.expList.isEmpty = TRUE;
  return node;
}


Absyn *newExpList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EXPLIST;
  node->line = -1;
  node->u.expList.isEmpty = FALSE;
  node->u.expList.head = head;
  node->u.expList.tail = tail;
  return node;
}


/**************************************************************/


static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}


static void say(char *s) {
  printf("%s", s);
}


static void sayInt(int i) {
  printf("%d", i);
}


static void sayBoolean(boolean b) {
  if (b) {
    printf("true");
  } else {
    printf("false");
  }
}


static void showNode(Absyn *node, int indent);


static void showNameTy(Absyn *node, int n) {
  indent(n);
  say("NameTy(");
  say(symToString(node->u.nameTy.name));
  say(")");
}


static void showArrayTy(Absyn *node, int n) {
  indent(n);
  say("ArrayTy(\n");
  indent(n + 1);
  sayInt(node->u.arrayTy.size);
  say(",\n");
  showNode(node->u.arrayTy.ty, n + 1);
  say(")");
}


static void showTypeDec(Absyn *node, int n) {
  indent(n);
  say("TypeDec(\n");
  indent(n + 1);
  say(symToString(node->u.typeDec.name));
  say(",\n");
  showNode(node->u.typeDec.ty, n + 1);
  say(")");
}


static void showProcDec(Absyn *node, int n) {
  indent(n);
  say("ProcDec(\n");
  indent(n + 1);
  say(symToString(node->u.procDec.name));
  say(",\n");
  showNode(node->u.procDec.params, n + 1);
  say(",\n");
  showNode(node->u.procDec.decls, n + 1);
  say(",\n");
  showNode(node->u.procDec.body, n + 1);
  say(")");
}


static void showParDec(Absyn *node, int n) {
  indent(n);
  say("ParDec(\n");
  indent(n + 1);
  say(symToString(node->u.parDec.name));
  say(",\n");
  showNode(node->u.parDec.ty, n + 1);
  say(",\n");
  indent(n + 1);
  sayBoolean(node->u.parDec.isRef);
  say(")");
}


static void showVarDec(Absyn *node, int n) {
  indent(n);
  say("VarDec(\n");
  indent(n + 1);
  say(symToString(node->u.varDec.name));
  say(",\n");
  showNode(node->u.varDec.ty, n + 1);
  say(")");
}


static void showEmptyStm(Absyn *node, int n) {
  indent(n);
  say("EmptyStm()");
}


static void showCompStm(Absyn *node, int n) {
  indent(n);
  say("CompStm(\n");
  showNode(node->u.compStm.stms, n + 1);
  say(")");
}


static void showAssignStm(Absyn *node, int n) {
  indent(n);
  say("AssignStm(\n");
  showNode(node->u.assignStm.var, n + 1);
  say(",\n");
  showNode(node->u.assignStm.exp, n + 1);
  say(")");
}


static void showIfStm(Absyn *node, int n) {
  indent(n);
  say("IfStm(\n");
  showNode(node->u.ifStm.test, n + 1);
  say(",\n");
  showNode(node->u.ifStm.thenPart, n + 1);
  say(",\n");
  showNode(node->u.ifStm.elsePart, n + 1);
  say(")");
}


static void showWhileStm(Absyn *node, int n) {
  indent(n);
  say("WhileStm(\n");
  showNode(node->u.whileStm.test, n + 1);
  say(",\n");
  showNode(node->u.whileStm.body, n + 1);
  say(")");
}


static void showCallStm(Absyn *node, int n) {
  indent(n);
  say("CallStm(\n");
  indent(n + 1);
  say(symToString(node->u.callStm.name));
  say(",\n");
  showNode(node->u.callStm.args, n + 1);
  say(")");
}


static void showOpExp(Absyn *node, int n) {
  indent(n);
  say("OpExp(\n");
  indent(n + 1);
  switch (node->u.opExp.op) {
    case ABSYN_OP_EQU:
      say("EQU");
      break;
    case ABSYN_OP_NEQ:
      say("NEQ");
      break;
    case ABSYN_OP_LST:
      say("LST");
      break;
    case ABSYN_OP_LSE:
      say("LSE");
      break;
    case ABSYN_OP_GRT:
      say("GRT");
      break;
    case ABSYN_OP_GRE:
      say("GRE");
      break;
    case ABSYN_OP_ADD:
      say("ADD");
      break;
    case ABSYN_OP_SUB:
      say("SUB");
      break;
    case ABSYN_OP_MUL:
      say("MUL");
      break;
    case ABSYN_OP_DIV:
      say("DIV");
      break;
    default:
      error("unknown operator %d in showOp", node->u.opExp.op);
  }
  say(",\n");
  showNode(node->u.opExp.left, n + 1);
  say(",\n");
  showNode(node->u.opExp.right, n + 1);
  say(")");
}


static void showVarExp(Absyn *node, int n) {
  indent(n);
  say("VarExp(\n");
  showNode(node->u.varExp.var, n + 1);
  say(")");
}


static void showIntExp(Absyn *node, int n) {
  indent(n);
  say("IntExp(");
  sayInt(node->u.intExp.val);
  say(")");
}


static void showSimpleVar(Absyn *node, int n) {
  indent(n);
  say("SimpleVar(");
  say(symToString(node->u.simpleVar.name));
  say(")");
}


static void showArrayVar(Absyn *node, int n) {
  indent(n);
  say("ArrayVar(\n");
  showNode(node->u.arrayVar.var, n + 1);
  say(",\n");
  showNode(node->u.arrayVar.index, n + 1);
  say(")");
}


static void showDecList(Absyn *node, int n) {
  indent(n);
  say("DecList(");
  while (!node->u.decList.isEmpty) {
    say("\n");
    showNode(node->u.decList.head, n + 1);
    node = node->u.decList.tail;
    if (!node->u.decList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showStmList(Absyn *node, int n) {
  indent(n);
  say("StmList(");
  while (!node->u.stmList.isEmpty) {
    say("\n");
    showNode(node->u.stmList.head, n + 1);
    node = node->u.stmList.tail;
    if (!node->u.stmList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showExpList(Absyn *node, int n) {
  indent(n);
  say("ExpList(");
  while (!node->u.expList.isEmpty) {
    say("\n");
    showNode(node->u.expList.head, n + 1);
    node = node->u.expList.tail;
    if (!node->u.expList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showNode(Absyn *node, int indent) {
  if (node == NULL) {
    error("showNode got NULL node pointer");
  }
  switch (node->type) {
    case ABSYN_NAMETY:
      showNameTy(node, indent);
      break;
    case ABSYN_ARRAYTY:
      showArrayTy(node, indent);
      break;
    case ABSYN_TYPEDEC:
      showTypeDec(node, indent);
      break;
    case ABSYN_PROCDEC:
      showProcDec(node, indent);
      break;
    case ABSYN_PARDEC:
      showParDec(node, indent);
      break;
    case ABSYN_VARDEC:
      showVarDec(node, indent);
      break;
    case ABSYN_EMPTYSTM:
      showEmptyStm(node, indent);
      break;
    case ABSYN_COMPSTM:
      showCompStm(node, indent);
      break;
    case ABSYN_ASSIGNSTM:
      showAssignStm(node, indent);
      break;
    case ABSYN_IFSTM:
      showIfStm(node, indent);
      break;
    case ABSYN_WHILESTM:
      showWhileStm(node, indent);
      break;
    case ABSYN_CALLSTM:
      showCallStm(node, indent);
      break;
    case ABSYN_OPEXP:
      showOpExp(node, indent);
      break;
    case ABSYN_VAREXP:
      showVarExp(node, indent);
      break;
    case ABSYN_INTEXP:
      showIntExp(node, indent);
      break;
    case ABSYN_SIMPLEVAR:
      showSimpleVar(node, indent);
      break;
    case ABSYN_ARRAYVAR:
      showArrayVar(node, indent);
      break;
    case ABSYN_DECLIST:
      showDecList(node, indent);
      break;
    case ABSYN_STMLIST:
      showStmList(node, indent);
      break;
    case ABSYN_EXPLIST:
      showExpList(node, indent);
      break;
    default:
      error("unknown node type %d in showAbsyn", node->type);
  }
}


void showAbsyn(Absyn *node) {
  showNode(node, 0);
  printf("\n");
}
