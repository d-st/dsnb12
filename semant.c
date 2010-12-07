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


// modulglobale (interne) primitive typen
Type *builtinType_int,
     *builtinType_bool;

Type *checkNameTy(Absyn *node, Table *symtab) {
  
  return NULL;
}

Type *checkArrayTy(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkVarDec(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkEmptyStm(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkCompStm(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkAssignStm(Absyn *node, Table *symtab) {
  Type *lhs, *rhs;

  lhs = checkNode(node->u.assignStm.var, symtab);
  rhs = checkNode(node->u.assignStm.exp, symtab);

  if(lhs != rhs)
    error("assignment has different types in line %d", node->line);
  return NULL;
}

Type *checkIfStm(Absyn *node, Table *symtab) {
  if(checkNode(node->u.ifStm.test, symtab) != builtinType_bool)
    error("'if' test expression must be of type boolean in line %d", node->line);
  checkNode(node->u.ifStm.thenPart, symtab);
  checkNode(node->u.ifStm.elsePart, symtab);
  return NULL;
}

Type *checkWhileStm(Absyn *node, Table *symtab) {
  if(checkNode(node->u.whileStm.test, symtab) != builtinType_bool)
    error("Error: 'while' test expression must be of type boolean in line %d", node->line);
  checkNode(node->u.whileStm.body, symtab);
  return NULL;
}

Type *checkCallStm(Absyn *node, Table *symtab) {
  Entry *entry;
  ParamTypes *p;
  Absyn *arg;
  int n = 0;
  
  entry = lookup(symtab, node->u.callStm.name);
  if(entry == NULL)
    error("undefined procedure '%s' in line %d", node->u.callStm.name, node->line); // TODO node->u.callStm.name is Sym* not char*
 if(entry->kind != ENTRY_KIND_PROC) 
    error("call of non-procedure '%s' in line %d", node->u.callStm.name, node->line);
	
  // TODO: check parameter types. 
  p = entry->u.procEntry.paramTypes;
  arg = node->u.callStm.args;
  
  while(!p->isEmpty) {
    n++;
	if(p->type != checkNode(soll, symtab))
	  error("procedure '%s' argument %d type mismatch in line %d", node->u.callStm.name, n, node->line);
	p = p->next;
  }
  if(n < soll_n)
    error("procedure '%s' called with too few arguments in line %d", node->u.callStm.name, node->line);
  if(n > soll_n)
    error("procedure '%s' called with too many arguments in line %d", node->u.callStm.name, node->line);
  
  // TODO: check ref params for being a variable explicitly.
	
  return NULL;
}

Type *checkVarExp(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkIntExp(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkStmList(Absyn *node, Table *symtab) {

  return NULL;
}

Type *checkExpList(Absyn *node, Table *symtab) {

  return NULL;
}


Type *checkOpExp(Absyn *node, Table *symtab) {
  Type *leftType, *rightType;

  leftType  = checkNode(node->u.opExp.left,  symtab);
  rightType = checkNode(node->u.opExp.right, symtab);

  if(leftType != rightType) 
    error("expression combines different types in line %d", node->line);
  switch(node->u.opExp.op) {
    case ABSYN_OP_EQU: // case fall-through, following cases same type
    case ABSYN_OP_NEQ:
    case ABSYN_OP_LST:
    case ABSYN_OP_LSE:
    case ABSYN_OP_GRT:
    case ABSYN_OP_GRE:
      if(leftType != builtinType_int) 
        error("comparison requires integer operands in line %d", node->line);
      node->type_t = builtinType_bool;
      break;
    case ABSYN_OP_ADD: // case fall-through, following cases same type
    case ABSYN_OP_SUB:
    case ABSYN_OP_MUL:
    case ABSYN_OP_DIV:
      if(leftType != builtinType_int) 
        error("comparison requires integer operands in line %d", node->line);
      node->type_t = builtinType_int;
      break;
    default:
      error("unknown node type %d in checkNode", node->type);
  }
  return node->type_t;
}

Type *checkSimpleVar(Absyn *node, Table *symtab) {
  Entry *entry;
  entry = lookup(symtab, node->u.simpleVar.name);
  if(entry == NULL) 
    error("undefined variable '%s' in line %d", node->u.simpleVar.name, node->line);
  if(entry->kind != ENTRY_KIND_VAR) 
    error("'%s' is not a variable in line %d", node->u.simpleVar.name, node->line);
  node->type_t = entry->u.varEntry.type;
  return node->type_t;
}

Type *checkArrayVar(Absyn *node, Table *symtab) {
  if(checkNode(node->u.arrayVar.index, symtab) != builtinType_int)
    error("illegal indexing with a non-integer in line %d", node->line);
  node->type_t = checkNode(node->u.arrayVar.var, symtab);
  return node->type_t;
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
/*
  ParamTypes *paramTypes, *next;
  Type *type;

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
/*
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
*/
  return NULL;
}

Type *checkDecList(Absyn *node, Table *symtab) {
  if(node->u.decList.isEmpty)
    return NULL;
  checkNode(node->u.decList.head, symtab);
  checkNode(node->u.decList.tail, symtab);
  return NULL;
}

Type *checkNode(Absyn *node, Table *symtab) {
  switch (node->type) {
    case ABSYN_NAMETY:
      checkNameTy(node, symtab);
      break;
    case ABSYN_ARRAYTY:
      checkArrayTy(node, symtab);
      break;
    case ABSYN_TYPEDEC:
      checkTypeDec(node, symtab);
      break;
    case ABSYN_PROCDEC:
      checkProcDec(node, symtab);
      break;
    case ABSYN_PARDEC:
      checkParDec(node, symtab);
      break;
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
    case ABSYN_DECLIST:
	  checkDecList(node, symtab);
      break;
    case ABSYN_STMLIST:
      checkStmList(node, symtab);
      break;
    case ABSYN_EXPLIST:
      checkExpList(node, symtab);
      break;
    default:
      error("unknown node type %d in checkNode", node->type);
  }

  return NULL;
}

Table *check(Absyn *program, boolean showSymbolTables) {
  Table *globalTable;
  globalTable = newTable(NULL);

  /* generate built-in types */
  builtinType_int  = newPrimitiveType("int");
  builtinType_bool = newPrimitiveType("bool");

  Entry *exit_builtin = newProcEntry(emptyParamTypes(), newTable(globalTable));
  Entry *time_builtin = newProcEntry(newParamTypes(builtinType_int, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *readi_builtin = newProcEntry(newParamTypes(builtinType_int, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *readc_builtin = newProcEntry(newParamTypes(builtinType_int, TRUE, emptyParamTypes()), newTable(globalTable));
  Entry *printi_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *printc_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *clearAll_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, emptyParamTypes()), newTable(globalTable));
  Entry *setPixel_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, emptyParamTypes()))), newTable(globalTable));
  Entry *drawCircle_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, emptyParamTypes())))), newTable(globalTable));
  Entry *drawLine_builtin = newProcEntry(newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, newParamTypes(builtinType_int, FALSE, emptyParamTypes()))))), newTable(globalTable));

  /* setup global symbol table */
  enter(globalTable, newSym("int"), newTypeEntry(builtinType_int));
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
