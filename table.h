/*
 * table.h -- symbol table
 */


#ifndef _TABLE_H_
#define _TABLE_H_


#define ENTRY_KIND_TYPE		0
#define ENTRY_KIND_VAR		1
#define ENTRY_KIND_PROC		2


typedef struct {
  int kind;
  union {
    struct {
      Type *type;
    } typeEntry;
    struct {
      Type *type;
      boolean isRef;
    } varEntry;
    struct {
      ParamTypes *paramTypes;
      struct table *localTable;
    } procEntry;
  } u;
} Entry;


typedef struct bintree {
  Sym *sym;
  unsigned key;
  Entry *entry;
  struct bintree *left;
  struct bintree *right;
} Bintree;


typedef struct table {
  Bintree *bintree;
  struct table *upperLevel;
} Table;


Entry *newTypeEntry(Type *type);
Entry *newVarEntry(Type *type, boolean isRef);
Entry *newProcEntry(ParamTypes *paramTypes, Table *localTable);

Table *newTable(Table *upperLevel);
Entry *enter(Table *table, Sym *sym, Entry *entry);
Entry *lookup(Table *table, Sym *sym);

void showEntry(Entry *entry);
void showTable(Table *table);


#endif /* _TABLE_H_ */
