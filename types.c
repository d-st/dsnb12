/*
 * types.c -- type representation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "types.h"


Type *newPrimitiveType(char *printName) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_PRIMITIVE;
  type->u.primitiveType.printName = printName;
  return type;
}


Type *newArrayType(int size, Type *baseType) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_ARRAY;
  type->u.arrayType.size = size;
  type->u.arrayType.baseType = baseType;
  return type;
}


ParamTypes *emptyParamTypes(void) {
  ParamTypes *paramTypes;

  paramTypes = (ParamTypes *) allocate(sizeof(ParamTypes));
  paramTypes->isEmpty = TRUE;
  return paramTypes;
}


ParamTypes *newParamTypes(Type *type, boolean isRef, ParamTypes *next) {
  ParamTypes *paramTypes;

  paramTypes = (ParamTypes *) allocate(sizeof(ParamTypes));
  paramTypes->isEmpty = FALSE;
  paramTypes->type = type;
  paramTypes->isRef = isRef;
  paramTypes->next = next;
  return paramTypes;
}


void showType(Type *type) {
  switch (type->kind) {
    case TYPE_KIND_PRIMITIVE:
      printf("%s", type->u.primitiveType.printName);
      break;
    case TYPE_KIND_ARRAY:
      printf("array [%d] of ", type->u.arrayType.size);
      showType(type->u.arrayType.baseType);
      break;
    default:
      error("unknown type kind %d in showType", type->kind);
  }
}


void showParamTypes(ParamTypes *paramTypes) {
  printf("(");
  while (!paramTypes->isEmpty) {
    if (paramTypes->isRef) {
      printf("ref ");
    }
    showType(paramTypes->type);
    paramTypes = paramTypes->next;
    if (!paramTypes->isEmpty) {
      printf(", ");
    }
  }
  printf(")");
}
