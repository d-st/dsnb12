/*
 * sym.h -- symbol management
 */


#ifndef _SYM_H_
#define _SYM_H_


#define INITIAL_HASH_SIZE	100	/* will be increased to next prime */


typedef struct sym {
  char *string;			/* external representation of symbol */
  unsigned stamp;		/* unique random stamp for external use */
  unsigned hashValue;		/* hash value of string, internal use */
  struct sym *next;		/* symbol chaining, internal use */
} Sym;


Sym *newSym(char *string);
char *symToString(Sym *sym);
unsigned symToStamp(Sym *sym);


#endif /* _SYM_H_ */
