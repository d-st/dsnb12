/*
 * absyn.h -- abstract syntax
 */


#ifndef _ABSYN_H_
#define _ABSYN_H_


#define ABSYN_NAMETY		0
#define ABSYN_ARRAYTY		1
#define ABSYN_TYPEDEC		2
#define ABSYN_PROCDEC		3
#define ABSYN_PARDEC		4
#define ABSYN_VARDEC		5
#define ABSYN_EMPTYSTM		6
#define ABSYN_COMPSTM		7
#define ABSYN_ASSIGNSTM		8
#define ABSYN_IFSTM		9
#define ABSYN_WHILESTM		10
#define ABSYN_CALLSTM		11
#define ABSYN_OPEXP		12
#define ABSYN_VAREXP		13
#define ABSYN_INTEXP		14
#define ABSYN_SIMPLEVAR		15
#define ABSYN_ARRAYVAR		16
#define ABSYN_DECLIST		17
#define ABSYN_STMLIST		18
#define ABSYN_EXPLIST		19

#define ABSYN_OP_EQU		0
#define ABSYN_OP_NEQ		1
#define ABSYN_OP_LST		2
#define ABSYN_OP_LSE		3
#define ABSYN_OP_GRT		4
#define ABSYN_OP_GRE		5
#define ABSYN_OP_ADD		6
#define ABSYN_OP_SUB		7
#define ABSYN_OP_MUL		8
#define ABSYN_OP_DIV		9


typedef struct absyn {
  int type;
  int line;
  union {
    struct {
      Sym *name;
    } nameTy;
    struct {
      int size;
      struct absyn *ty;
    } arrayTy;
    struct {
      Sym *name;
      struct absyn *ty;
    } typeDec;
    struct {
      Sym *name;
      struct absyn *params;
      struct absyn *decls;
      struct absyn *body;
    } procDec;
    struct {
      Sym *name;
      struct absyn *ty;
      boolean isRef;
    } parDec;
    struct {
      Sym *name;
      struct absyn *ty;
    } varDec;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } emptyStm;
    struct {
      struct absyn *stms;
    } compStm;
    struct {
      struct absyn *var;
      struct absyn *exp;
    } assignStm;
    struct {
      struct absyn *test;
      struct absyn *thenPart;
      struct absyn *elsePart;
    } ifStm;
    struct {
      struct absyn *test;
      struct absyn *body;
    } whileStm;
    struct {
      Sym *name;
      struct absyn *args;
    } callStm;
    struct {
      int op;
      struct absyn *left;
      struct absyn *right;
    } opExp;
    struct {
      struct absyn *var;
    } varExp;
    struct {
      int val;
    } intExp;
    struct {
      Sym *name;
    } simpleVar;
    struct {
      struct absyn *var;
      struct absyn *index;
    } arrayVar;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } decList;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } stmList;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } expList;
  } u;
} Absyn;


Absyn *newNameTy(int line, Sym *name);
Absyn *newArrayTy(int line, int size, Absyn *ty);
Absyn *newTypeDec(int line, Sym *name, Absyn *ty);
Absyn *newProcDec(int line, Sym *name,
                  Absyn *params, Absyn *decls, Absyn *body);
Absyn *newParDec(int line, Sym *name, Absyn *ty, boolean isRef);
Absyn *newVarDec(int line, Sym *name, Absyn *ty);
Absyn *newEmptyStm(int line);
Absyn *newCompStm(int line, Absyn *stms);
Absyn *newAssignStm(int line, Absyn *var, Absyn *exp);
Absyn *newIfStm(int line, Absyn *test, Absyn *thenPart, Absyn *elsePart);
Absyn *newWhileStm(int line, Absyn *test, Absyn *body);
Absyn *newCallStm(int line, Sym *name, Absyn *args);
Absyn *newOpExp(int line, int op, Absyn *left, Absyn *right);
Absyn *newVarExp(int line, Absyn *var);
Absyn *newIntExp(int line, int val);
Absyn *newSimpleVar(int line, Sym *name);
Absyn *newArrayVar(int line, Absyn *var, Absyn *index);
Absyn *emptyDecList(void);
Absyn *newDecList(Absyn *head, Absyn *tail);
Absyn *emptyStmList(void);
Absyn *newStmList(Absyn *head, Absyn *tail);
Absyn *emptyExpList(void);
Absyn *newExpList(Absyn *head, Absyn *tail);

void showAbsyn(Absyn *node);


#endif /* _ABSYN_H_ */
