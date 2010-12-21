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

boolean __debug = FALSE;

// modulglobale (interne) primitive typen
Type *builtinType_int, *builtinType_bool;

Type *checkNameTy(Absyn * node, Table * symtab)
{
	Entry *entry;
	entry = lookup(symtab, node->u.nameTy.name);
	if (entry == NULL) {
		error("undefined type '%s' in line %d",
		      symToString(node->u.nameTy.name), node->line);
	}
	// check if entry is type
	if (entry->kind != ENTRY_KIND_TYPE)
		error("'%s' is not a type in line %d",
		      symToString(node->u.nameTy.name), node->line);

	return entry->u.typeEntry.type;
}

Type *checkArrayTy(Absyn * node, Table * symtab)
{
	Type *baseType;
	baseType = checkNode(node->u.arrayTy.ty, symtab);
	return newArrayType(node->u.arrayTy.size, baseType);
}

Type *checkEmptyStm(Absyn * node, Table * symtab)
{
	// nothing to do: empty statement
	return NULL;
}

Type *checkCompStm(Absyn * node, Table * symtab)
{
	return checkNode(node->u.compStm.stms, symtab);
}

Type *checkAssignStm(Absyn * node, Table * symtab)
{
	Type *lhs, *rhs;

	lhs = checkNode(node->u.assignStm.var, symtab);
	rhs = checkNode(node->u.assignStm.exp, symtab);
	if (lhs != rhs)
		error("assignment has different types in line %d", node->line);
	if (lhs != builtinType_int)
		error("assignment requires integer variable in line %d",
		      node->line);
	return NULL;
}

Type *checkIfStm(Absyn * node, Table * symtab)
{
	if (checkNode(node->u.ifStm.test, symtab) != builtinType_bool)
		error("'if' test expression must be of type boolean in line %d",
		      node->line);
	checkNode(node->u.ifStm.thenPart, symtab);
	checkNode(node->u.ifStm.elsePart, symtab);
	return NULL;
}

Type *checkWhileStm(Absyn * node, Table * symtab)
{
	if (checkNode(node->u.whileStm.test, symtab) != builtinType_bool)
		error
		    ("'while' test expression must be of type boolean in line %d",
		     node->line);
	checkNode(node->u.whileStm.body, symtab);
	return NULL;
}

Type *checkCallStm(Absyn * node, Table * symtab)
{
	Entry *entry;
	ParamTypes *p;
	Absyn *args, *arg;
	int n = 0;

	entry = lookup(symtab, node->u.callStm.name);
	if (entry == NULL)
		error("undefined procedure '%s' in line %d",
		      symToString(node->u.callStm.name), node->line);
	if (entry->kind != ENTRY_KIND_PROC)
		error("call of non-procedure '%s' in line %d",
		      symToString(node->u.callStm.name), node->line);

	p = entry->u.procEntry.paramTypes;
	args = node->u.callStm.args;
	if (args->type != ABSYN_EXPLIST)
		error
		    ("error in abstract syntax. should not happen; internal compiler error.");
	arg = args;

	while (p && !p->isEmpty) {
		n++;
		if (arg->u.expList.isEmpty)	// (n < soll_n)
			error
			    ("procedure '%s' called with too few arguments in line %d",
			     symToString(node->u.callStm.name), node->line);
		if (p->type != checkNode(arg->u.expList.head, symtab)) {
			error
			    ("procedure '%s' argument %d type mismatch in line %d",
			     symToString(node->u.callStm.name), n, node->line);
		}

		arg = arg->u.expList.tail;
		p = p->next;
	}
	if (!(arg->u.expList.isEmpty))	// (n > soll_n)
		error
		    ("procedure '%s' called with too many arguments in line %d",
		     symToString(node->u.callStm.name), node->line);

	// check ref params for being a variable explicitly.
	arg = args;
	p = entry->u.procEntry.paramTypes;
	while (p && !p->isEmpty) {
		if (p->isRef && arg->u.expList.head->type != ABSYN_VAREXP) {
			error
			    ("procedure '%s' argument %d must be a variable in line %d",
			     symToString(node->u.callStm.name), n, node->line);
		}
		arg = arg->u.expList.tail;
		p = p->next;
	}

	return NULL;
}

Type *checkVarExp(Absyn * node, Table * symtab)
{
	return checkNode(node->u.varExp.var, symtab);
}

Type *checkIntExp(Absyn * node, Table * symtab)
{
	return builtinType_int;
}

Type *checkStmList(Absyn * node, Table * symtab)
{
	if (node->u.stmList.isEmpty)
		return NULL;
	checkNode(node->u.stmList.head, symtab);
	checkNode(node->u.stmList.tail, symtab);
	return NULL;
}

Type *checkExpList(Absyn * node, Table * symtab)
{
	if (node->u.expList.isEmpty)
		return NULL;
	checkNode(node->u.expList.head, symtab);
	checkNode(node->u.expList.tail, symtab);
	return NULL;
}

Type *checkOpExp(Absyn * node, Table * symtab)
{
	Type *leftType, *rightType;

	leftType = checkNode(node->u.opExp.left, symtab);
	rightType = checkNode(node->u.opExp.right, symtab);

	if (leftType != rightType)
		error("expression combines different types in line %d",
		      node->line);
	switch (node->u.opExp.op) {
	case ABSYN_OP_EQU:	// case fall-through, following cases same type
	case ABSYN_OP_NEQ:
	case ABSYN_OP_LST:
	case ABSYN_OP_LSE:
	case ABSYN_OP_GRT:
	case ABSYN_OP_GRE:
		if (leftType != builtinType_int)
			error("comparison requires integer operands in line %d",
			      node->line);
		node->type_t = builtinType_bool;
		break;
	case ABSYN_OP_ADD:	// case fall-through, following cases same type
	case ABSYN_OP_SUB:
	case ABSYN_OP_MUL:
	case ABSYN_OP_DIV:
		if (leftType != builtinType_int)
			error("comparison requires integer operands in line %d",
			      node->line);
		node->type_t = builtinType_int;
		break;
	default:
		error("unknown node type %d in checkNode", node->type);
	}
	return node->type_t;
}

Type *checkSimpleVar(Absyn * node, Table * symtab)
{
	Entry *entry;
	entry = lookup(symtab, node->u.simpleVar.name);
	if (entry == NULL)
		error("undefined variable '%s' in line %d",
		      symToString(node->u.simpleVar.name), node->line);
	if (entry->kind != ENTRY_KIND_VAR)
		error("'%s' is not a variable in line %d",
		      symToString(node->u.simpleVar.name), node->line);
	node->type_t = entry->u.varEntry.type;
	return node->type_t;
}

Type *checkArrayVar(Absyn * node, Table * symtab)
{
	Type *tmp;
	if (checkNode(node->u.arrayVar.index, symtab) != builtinType_int)
		error("illegal indexing with a non-integer in line %d",
		      node->line);
	tmp = checkNode(node->u.arrayVar.var, symtab);
	// check that tmp is an array
	if (tmp->kind != TYPE_KIND_ARRAY)
		error("illegal indexing a non-array in line %d", node->line);

	node->type_t = tmp->u.arrayType.baseType;
	return node->type_t;
}

Type *checkTypeDec(Absyn * node, Table * symtab)
{
	Type *type;
	Entry *entry;
	type = checkNode(node->u.typeDec.ty, symtab);
	entry = newTypeEntry(type);
	if (enter(symtab, node->u.typeDec.name, entry) == NULL) {
		error("redeclaration of '%s' in line %d",
		      symToString(node->u.typeDec.name), node->line);
	}
	return NULL;
}

Type *checkVarDec(Absyn * node, Table * symtab)
{
	Type *type;
	Entry *entry;
	type = checkNode(node->u.typeDec.ty, symtab);
	entry = newVarEntry(type, FALSE);
	if (enter(symtab, node->u.typeDec.name, entry) == NULL) {
		error("redeclaration of '%s' in line %d",
		      symToString(node->u.typeDec.name), node->line);
	}
	return NULL;
}

Type *checkParDec(Absyn * node, Table * symtab)
{
	return checkNode(node->u.parDec.ty, symtab);
}

Type *checkProcDec(Absyn * node, Table * symtab)
{
	Table *localSymtab;
	Entry *entry;

	// loopup procs local symtable here
	entry = lookup(symtab, node->u.procDec.name);
	localSymtab = entry->u.procEntry.localTable;

	// compute local declarations
	checkNode(node->u.procDec.decls, localSymtab);

	// compute procedure body
	checkNode(node->u.procDec.body, localSymtab);

	// optional debug output
	if (__debug) {
		printf("symbol table at end of procedure '%s':\n",
		       symToString(node->u.procDec.name));
		showTable(localSymtab);
	}

	return NULL;
}

Type *checkDecList(Absyn * node, Table * symtab)
{
	if (node->u.decList.isEmpty)
		return NULL;
	checkNode(node->u.decList.head, symtab);
	checkNode(node->u.decList.tail, symtab);
	return NULL;
}

Type *checkNode(Absyn * node, Table * symtab)
{
	switch (node->type) {
	case ABSYN_NAMETY:
		return checkNameTy(node, symtab);
		break;
	case ABSYN_ARRAYTY:
		return checkArrayTy(node, symtab);
		break;
	case ABSYN_TYPEDEC:
		// already done in 1st run
		break;
	case ABSYN_PROCDEC:
		return checkProcDec(node, symtab);
		break;
	case ABSYN_PARDEC:
		return checkParDec(node, symtab);
		break;
	case ABSYN_VARDEC:
		return checkVarDec(node, symtab);
		break;
	case ABSYN_EMPTYSTM:
		return checkEmptyStm(node, symtab);
		break;
	case ABSYN_COMPSTM:
		return checkCompStm(node, symtab);
		break;
	case ABSYN_ASSIGNSTM:
		return checkAssignStm(node, symtab);
		break;
	case ABSYN_IFSTM:
		return checkIfStm(node, symtab);
		break;
	case ABSYN_WHILESTM:
		return checkWhileStm(node, symtab);
		break;
	case ABSYN_CALLSTM:
		return checkCallStm(node, symtab);
		break;
	case ABSYN_OPEXP:
		return checkOpExp(node, symtab);
		break;
	case ABSYN_VAREXP:
		return checkVarExp(node, symtab);
		break;
	case ABSYN_INTEXP:
		return checkIntExp(node, symtab);
		break;
	case ABSYN_SIMPLEVAR:
		return checkSimpleVar(node, symtab);
		break;
	case ABSYN_ARRAYVAR:
		return checkArrayVar(node, symtab);
		break;
	case ABSYN_DECLIST:
		return checkDecList(node, symtab);
		break;
	case ABSYN_STMLIST:
		return checkStmList(node, symtab);
		break;
	case ABSYN_EXPLIST:
		return checkExpList(node, symtab);
		break;
	default:
		error("unknown node type %d in checkNode, line %d", node->type,
		      node->line);
	}

	return NULL;
}

void enter_proc_decs(Absyn * program, Table * symtab)
{
	ParamTypes *paramTypes, *p_last, *p_new;
	Table *localSymtab;
	Entry *entry;
	Absyn *tmp, *node;
	Sym *name;

	while (!program->u.decList.isEmpty) {
		node = program->u.decList.head;
		if (node->type == ABSYN_PROCDEC) {

			// determine parameter types
			paramTypes = NULL;
			tmp = node->u.procDec.params;
			if (tmp->u.decList.isEmpty)
				paramTypes = emptyParamTypes();
			else {
				while (!(tmp->u.decList.isEmpty)) {
					p_new =
					    newParamTypes(checkNode
							  (tmp->u.decList.head,
							   symtab),
							  tmp->u.decList.
							  head->u.parDec.isRef,
							  NULL);
					if (paramTypes == NULL)
						paramTypes = p_new;
					else
						p_last->next = p_new;
					p_last = p_new;
					tmp = tmp->u.decList.tail;
				}
				p_last->next = emptyParamTypes();
			}

			// enter proc into global symbol table
			localSymtab = newTable(symtab);
			entry = newProcEntry(paramTypes, localSymtab);

			if (enter(symtab, node->u.procDec.name, entry) == NULL) {
				error("redeclaration of '%s' in line %d",
				      symToString(node->u.procDec.name),
				      node->line);
			}
			// enter parameters into local symbol table
			tmp = node->u.procDec.params;
			while (!(tmp->u.decList.isEmpty)) {
				name = tmp->u.decList.head->u.varDec.name;
				entry =
				    newVarEntry(checkNode
						(tmp->u.decList.head->u.
						 parDec.ty, symtab),
						tmp->u.decList.head->u.
						parDec.isRef);
				enter(localSymtab, name, entry);
				tmp = tmp->u.decList.tail;
			}

		}
		if (node->type == ABSYN_TYPEDEC) {
			checkTypeDec(node, symtab);
		}
		program = program->u.decList.tail;
	}
	return;
}

Table *check(Absyn * program, boolean showSymbolTables)
{
	Table *globalTable;
	Entry *e;
	globalTable = newTable(NULL);

	if (showSymbolTables) {
		__debug = TRUE;
	}

	/* generate built-in types */
	builtinType_int = newPrimitiveType("int");
	builtinType_bool = newPrimitiveType("bool");

	Entry *builtinProc_exit =
	    newProcEntry(emptyParamTypes(), newTable(globalTable));
	Entry *builtinProc_time =
	    newProcEntry(newParamTypes
			 (builtinType_int, TRUE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_readi =
	    newProcEntry(newParamTypes
			 (builtinType_int, TRUE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_readc =
	    newProcEntry(newParamTypes
			 (builtinType_int, TRUE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_printi =
	    newProcEntry(newParamTypes
			 (builtinType_int, FALSE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_printc =
	    newProcEntry(newParamTypes
			 (builtinType_int, FALSE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_clearAll =
	    newProcEntry(newParamTypes
			 (builtinType_int, FALSE, emptyParamTypes()),
			 newTable(globalTable));
	Entry *builtinProc_setPixel =
	    newProcEntry(newParamTypes(builtinType_int, FALSE,
				       newParamTypes(builtinType_int, FALSE,
						     newParamTypes
						     (builtinType_int, FALSE,
						      emptyParamTypes()))),
			 newTable(globalTable));
	Entry *builtinProc_drawCircle =
	    newProcEntry(newParamTypes(builtinType_int, FALSE,
				       newParamTypes(builtinType_int, FALSE,
						     newParamTypes
						     (builtinType_int, FALSE,
						      newParamTypes
						      (builtinType_int, FALSE,
						       emptyParamTypes())))),
			 newTable(globalTable));
	Entry *builtinProc_drawLine =
	    newProcEntry(newParamTypes(builtinType_int, FALSE,
				       newParamTypes(builtinType_int, FALSE,
						     newParamTypes
						     (builtinType_int, FALSE,
						      newParamTypes
						      (builtinType_int, FALSE,
						       newParamTypes
						       (builtinType_int, FALSE,
							emptyParamTypes()))))),
			 newTable(globalTable));

	/* setup global symbol table */
	enter(globalTable, newSym("int"), newTypeEntry(builtinType_int));
	enter(globalTable, newSym("exit"), builtinProc_exit);
	enter(globalTable, newSym("time"), builtinProc_time);
	enter(globalTable, newSym("readi"), builtinProc_readi);
	enter(globalTable, newSym("readc"), builtinProc_readc);
	enter(globalTable, newSym("printi"), builtinProc_printi);
	enter(globalTable, newSym("printc"), builtinProc_printc);
	enter(globalTable, newSym("clearAll"), builtinProc_clearAll);
	enter(globalTable, newSym("setPixel"), builtinProc_setPixel);
	enter(globalTable, newSym("drawCircle"), builtinProc_drawCircle);
	enter(globalTable, newSym("drawLine"), builtinProc_drawLine);

	/* enter all procs into global symtable */
	enter_proc_decs(program, globalTable);

	/* do semantic checks */
	checkNode(program, globalTable);

	/* check if "main()" is present */
	e = lookup(globalTable, newSym("main"));
	if (e == NULL)
		error("procedure 'main' is missing");
	if (e->kind != ENTRY_KIND_PROC)
		error("'main' is not a procedure");
	// check that main has no params
	if (!e->u.procEntry.paramTypes->isEmpty)
		error("procedure 'main' must not have any parameters");

	/* return global symbol table */
	return globalTable;
}
