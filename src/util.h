#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

#define perr(msg, ...) { fprintf(stderr, msg, ##__VA_ARGS__); puts(""); exit(-1); }

#define FUNC_ID(_def) (_def->funcdef.v->id)

extern void prepTable(void);
extern int push2Table(void *, int);
extern void dumpTable(int);
extern void destroyState(void);

extern void *myrealloc(void *, size_t);
extern void *allocstring(size_t);

extern int isVoid(Type *);
extern int isFlt(Type *);
extern int isInt(Type *);
extern int isChar(Type *);
extern int isArray(Type *);
extern Type *getrefType(RefVar *);
extern Type *expType(Exp *);
extern int arrayDepth(Type *);

#endif
