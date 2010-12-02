/*
 * table.c -- symbol table
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "table.h"


Entry *newTypeEntry(Type *type) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_TYPE;
  entry->u.typeEntry.type = type;
  return entry;
}


Entry *newVarEntry(Type *type, boolean isRef) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_VAR;
  entry->u.varEntry.type = type;
  entry->u.varEntry.isRef = isRef;
  return entry;
}


Entry *newProcEntry(ParamTypes *paramTypes, Table *localTable) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_PROC;
  entry->u.procEntry.paramTypes = paramTypes;
  entry->u.procEntry.localTable = localTable;
  return entry;
}


Table *newTable(Table *upperLevel) {
  Table *table;

  table = (Table *) allocate(sizeof(Table));
  table->bintree = NULL;
  table->upperLevel = upperLevel;
  return table;
}


Entry *enter(Table *table, Sym *sym, Entry *entry) {
  unsigned key;
  Bintree *newtree;
  Bintree *current;
  Bintree *previous;

  key = symToStamp(sym);
  newtree = (Bintree *) allocate(sizeof(Bintree));
  newtree->sym = sym;
  newtree->key = key;
  newtree->entry = entry;
  newtree->left = NULL;
  newtree->right = NULL;
  if (table->bintree == NULL) {
    table->bintree = newtree;
  } else {
    current = table->bintree;
    while (1) {
      if (current->key == key) {
        /* symbol already in table */
        return NULL;
      }
      previous = current;
      if (current->key > key) {
        current = current->left;
      } else {
        current = current->right;
      }
      if (current == NULL) {
        if (previous->key > key) {
          previous->left = newtree;
        } else {
          previous->right = newtree;
        }
        break;
      }
    }
  }
  return entry;
}


static Entry *lookupBintree(Bintree *bintree, unsigned key) {
  while (bintree != NULL) {
    if (bintree->key == key) {
      return bintree->entry;
    }
    if (bintree->key > key) {
      bintree = bintree->left;
    } else {
      bintree = bintree->right;
    }
  }
  return NULL;
}


Entry *lookup(Table *table, Sym *sym) {
  unsigned key;
  Entry *entry;

  key = symToStamp(sym);
  while (table != NULL) {
    entry = lookupBintree(table->bintree, key);
    if (entry != NULL) {
      return entry;
    }
    table = table->upperLevel;
  }
  return NULL;
}


void showEntry(Entry *entry) {
  switch (entry->kind) {
    case ENTRY_KIND_TYPE:
      printf("type: ");
      showType(entry->u.typeEntry.type);
      break;
    case ENTRY_KIND_VAR:
      printf("var: ");
      if (entry->u.varEntry.isRef) {
        printf("ref ");
      }
      showType(entry->u.varEntry.type);
      break;
    case ENTRY_KIND_PROC:
      printf("proc: ");
      showParamTypes(entry->u.procEntry.paramTypes);
      break;
    default:
      error("unknown entry kind %d in showEntry", entry->kind);
  }
  printf("\n");
}


static void showBintree(Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  showBintree(bintree->left);
  printf("  %-10s --> ", symToString(bintree->sym));
  showEntry(bintree->entry);
  showBintree(bintree->right);
}


void showTable(Table *table) {
  int level;

  level = 0;
  while (table != NULL) {
    printf("  level %d\n", level);
    showBintree(table->bintree);
    table = table->upperLevel;
    level++;
  }
}
