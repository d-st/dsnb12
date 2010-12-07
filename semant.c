/*
 * semant.c -- semantic analysis
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
#include "semant.h"

Type *checkOp(Absyn *node, Table *symtab) {
  Type *leftType, *rightType, *type;

  leftType  = checkNode(node->u.opExp.left,  symtab);
  rightType = checkNode(node->u.opExp.right, symtab);
  type = NULL;

  if(leftType != rightType) error("expression combines different types in line %d", node->line);
  switch(node->u.opExp.op) {
    case ABSYN_OP_ADD:
/*
      if(leftType != int_builtin) error(""); // TODO
      type = int_builtin;
*/
      break;
    case ABSYN_OP_SUB:
      break;
    case ABSYN_OP_MUL:
      break;
    case ABSYN_OP_DIV:
      break;
    default:
      error("unknown node type %d in checkNode", node->type);
  }
  return type;
}

Type *checkSimpleVar(Absyn *node, Table *symtab) {
  Entry *entry;
  entry = lookup(symtab, node->u.simpleVar.name);
  if(entry == NULL) error("undefined variable '%s' in line %d", node->u.simpleVar.name, node->line);
  if(entry->kind != ENTRY_KIND_VAR) error("'%s' is not a variable in line %d", node->u.simpleVar.name, node->line);
  return entry->u.varEntry.type;
}

Type *checkTypeDec(Absyn *node, Table *symtab) {
  Type *type;
  Entry *entry;
  type = checkNode(node->u.typeDec.ty, symtab);
  entry = newTypeEntry(type);
  if(enter(symtab, node->u.typeDec.name, entry) == NULL) {
    error("redeclaration of '%s' in line %d", node->u.typeDec.name, node->line);
  }
  return NULL;
}

Type *checkParDec(Absyn *node, Table *symtab) {
  ParamTypes *paramTypes, *next;
  Type *type;
/*
  return emptyParamTypes();
  type = checkNode(node->u.parDec.ty, symtab);
  next = NULL; // TODO
  paramTypes = newParamTypes(type, node->u.parDec.isRef, next);
  
  return paramTypes;
  
  error("should never reach this");
*/
  return NULL;
}

Type *checkProcDec(Absyn *node, Table *symtab) {
  ParamTypes *paramTypes;
  Table *localSymtab;
  Entry *entry;

  localSymtab = newTable(symtab);
  paramTypes = checkNode(node->u.procDec.params, localSymtab); // TODO: ParamTypes* = Type*
  entry = newProcEntry(paramTypes, localSymtab);
  if(enter(symtab, node->u.procDec.name, entry) == NULL) {
    error("redeclaration of '%s' in line %d", node->u.procDec.name, node->line);
  }
  // TODO: eval node->u.procDec.decls
  // TODO: eval node->u.procDec.body

  return NULL;
}

Type *checkNode(Absyn *node, Table *symtab) {
  switch (node->type) {
/*
    case ABSYN_NAMETY:
      checkNameTy(node, symtab);
      break;
    case ABSYN_ARRAYTY:
      checkArrayTy(node, symtab);
      break;
*/
    case ABSYN_TYPEDEC:
      checkTypeDec(node, symtab);
      break;
    case ABSYN_PROCDEC:
      checkProcDec(node, symtab);
      break;
    case ABSYN_PARDEC:
      checkParDec(node, symtab);
      break;
/*
    case ABSYN_VARDEC:
      checkVarDec(node, symtab);
      break;
    case ABSYN_EMPTYSTM:
      checkEmptyStm(node, symtab);
      break;
    case ABSYN_COMPSTM:
      checkCompStm(node, symtab);
      break;
    case ABSYN_ASSIGNSTM:
      checkAssignStm(node, symtab);
      break;
    case ABSYN_IFSTM:
      checkIfStm(node, symtab);
      break;
    case ABSYN_WHILESTM:
      checkWhileStm(node, symtab);
      break;
    case ABSYN_CALLSTM:
      checkCallStm(node, symtab);
      break;
    case ABSYN_OPEXP:
      checkOpExp(node, symtab);
      break;
    case ABSYN_VAREXP:
      checkVarExp(node, symtab);
      break;
    case ABSYN_INTEXP:
      checkIntExp(node, symtab);
      break;
    case ABSYN_SIMPLEVAR:
      checkSimpleVar(node, symtab);
      break;
    case ABSYN_ARRAYVAR:
      checkArrayVar(node, symtab);
      break;
*/
    case ABSYN_DECLIST:
      if(node->u.decList.isEmpty) break;
      checkNode(node->u.decList.head, symtab);
      checkNode(node->u.decList.tail, symtab);
      break;
/*
    case ABSYN_STMLIST:
      checkStmList(node, symtab);
      break;
    case ABSYN_EXPLIST:
      checkExpList(node, symtab);
      break;
*/
    default:
      error("unknown node type %d in checkNode", node->type);
  }

  return NULL;
}

Table *check(Absyn *program, boolean showSymbolTables) {
  Table *globalTable;
  globalTable = newTable(NULL);

  /* generate built-in types */
  Type *int_builtin = newPrimitiveType("int");
  Entry *exit_builtin = newProcEntry(emptyParamTypes(), newTable(globalTable));
  Entry *time_builtin = newProcEntry(newParamTypes(int_builtin, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *readi_builtin = newProcEntry(newParamTypes(int_builtin, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *readc_builtin = newProcEntry(newParamTypes(int_builtin, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *printi_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *printc_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *clearAll_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *setPixel_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, emptyParamTypes()))), newTable(globalTable));
  Entry *drawCircle_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, emptyParamTypes())))), newTable(globalTable));
  Entry *drawLine_builtin = newProcEntry(newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, newParamTypes(int_builtin, FALSE, emptyParamTypes()))))), newTable(globalTable));

  /* setup global symbol table */
  enter(globalTable, newSym("int"), newTypeEntry(int_builtin));
  enter(globalTable, newSym("exit"), exit_builtin);
  enter(globalTable, newSym("time"), time_builtin);
  enter(globalTable, newSym("readi"), readi_builtin);
  enter(globalTable, newSym("readc"), readc_builtin);
  enter(globalTable, newSym("printi"), printi_builtin);
  enter(globalTable, newSym("printc"), printc_builtin);
  enter(globalTable, newSym("clearAll"), clearAll_builtin);
  enter(globalTable, newSym("setPixel"), setPixel_builtin);
  enter(globalTable, newSym("drawCircle"), drawCircle_builtin);
  enter(globalTable, newSym("drawLine"), drawLine_builtin);

  /* do semantic checks */
  checkNode(program, globalTable);

  /* check if "main()" is present */


  /* return global symbol table */
  if(showSymbolTables) {
    showTable(globalTable);
  }

  return globalTable;
}
