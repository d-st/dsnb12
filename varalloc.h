/*
 * varalloc.h -- variable allocation
 */


#ifndef _VARALLOC_H_
#define _VARALLOC_H_


#define INT_BYTE_SIZE	4	/* size of an int in bytes */
#define BOOL_BYTE_SIZE	4	/* size of a bool in bytes */
#define REF_BYTE_SIZE	4	/* size of an address in bytes */


void allocVars(Absyn *program, Table *globalTable, boolean showVarAlloc);


#endif /* _VARALLOC_H_ */
