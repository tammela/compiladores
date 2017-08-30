#ifndef _TK_H
#define _TK_H

#include <limits.h>

typedef enum {
/* type tokens */
   TK_INT = CHAR_MAX + 1,
   TK_FLOAT,
   TK_CHAR,
   TK_VOID,
/* command tokens */
   TK_IF,
   TK_ELSE,
   TK_WHILE,
   TK_RETURN,
   TK_NEW,
   TK_AS,
/* boolean tokens */
   TK_EQUALS,
   TK_GEQUALS,
   TK_LEQUALS,
   TK_NEQUALS,
   TK_AND,
   TK_OR,
/* print token */
   TK_PRINT,
/* id token */
   TK_ID,
/* semantic tokens */
   TK_STRING,
   TK_SINT,
   TK_SFLOAT,
   TK_LITERAL
} TK_ENUM;

typedef union {
   float fnum;
   int inum;
   char *str;
} SValue;

/* defined in main.c */
extern SValue sem;

#endif
