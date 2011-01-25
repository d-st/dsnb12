/*
 * types.h -- type representation
 */


#ifndef _TYPES_H_
#define _TYPES_H_


#define TYPE_KIND_PRIMITIVE	0
#define TYPE_KIND_ARRAY		1


typedef struct type {
  int kind;
  int size;
  union {
    struct {
      char *printName;
    } primitiveType;
    struct {
      int size;
      struct type *baseType;
    } arrayType;
  } u;
} Type;


typedef struct paramtypes {
  boolean isEmpty;
  Type *type;
  boolean isRef;
  int offset;
  struct paramtypes *next;
} ParamTypes;


Type *newPrimitiveType(char *printName, int size);
Type *newArrayType(int size, Type *baseType);

ParamTypes *emptyParamTypes(void);
ParamTypes *newParamTypes(Type *type, boolean isRef, ParamTypes *next);

void showType(Type *type);
void showParamTypes(ParamTypes *paramTypes);


#endif /* _TYPES_H_ */
