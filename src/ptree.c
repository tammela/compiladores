#include <stdio.h>
#include "tree.h"

static void printDef(int, Def *);
static void printExp(int, Exp *);
static void printStat(int, Stat *);

void printTree(Def *tree)
{
   printDef(2, tree);
}

static void printSpaces(int n)
{
   printf("\n");
   for (; n >= 0; n--)
      printf(" ");
}

static void printatomType(TYPE_tag t)
{
   switch (t) {
      case INT:
         printf("int");
         break;
      case FLOAT:
         printf("float");
         break;
      case CHAR:
         printf("char");
         break;
      case VOID:
         printf("void");
         break;
      case ARRAY:
         printf("[]");
         break;
   }
}

static void printType(Type *st)
{
   if (st == NULL) {
      return;
   }

   switch (st->tag) {
      case SEQ:
         printType(st->seqtype.left);
         printType(st->seqtype.right);
         break;
      case ATOMIC:
         printatomType(st->atomtype.t);
         break;
   }
}

static void printVar(Var *v)
{
   printf("%s ", v->id);
   printf("<");
   printType(v->type);
   printf("> ");
}

static void printDef(int n, Def *d)
{
   if (d == NULL) {
      return;
   }

   printSpaces(n);

   switch(d->tag) {
      case DEF_VAR:
         printf("<var>");
         printVar(d->vardef.v);
         break;
      case DEF_FUNCTION:
         printf("<func>");
         printVar(d->funcdef.v);
         printf(" (");
         printDef(n, d->funcdef.p);
         printf(")");
         printStat(n, d->funcdef.b);
         break;
      case DEF_SEQ:
         printDef(n, d->seqdef.left);
         printDef(n, d->seqdef.right);
         break;
   }
}

static void printBlock(int n, Block *b)
{
   if (b == NULL) {
      return;
   }

   if (b->v == NULL && b->s == NULL) {
      printf("{}");
      return;
   }

   printDef(n + 2, b->v);
   printStat(n + 2, b->s);
}

static void printRefVar(int n, RefVar *r)
{
   if (r == NULL) {
      return;
   }

   switch (r->tag) {
      case REF_VAR:
         printVar(r->refv.v);
         break;
      case REF_ARRAY:
         printExp(n + 2, r->refa.v);
         printf("[");
         printExp(n + 2, r->refa.idx);
         printf("]");
         break;
   }
}

static void printCall(int n, Call *c)
{
   printVar(c->v);
   printExp(n, c->e);
}

static char *opExp(EXP_tag tag)
{
   switch (tag) {
      case EXP_ADD:
         return "+";
      case EXP_SUB:
         return "-";
      case EXP_MUL:
         return "*";
      case EXP_DIV:
         return "/";
      case EXP_EQ:
         return "==";
      case EXP_NEQ:
         return "~=";
      case EXP_LEQ:
         return "<=";
      case EXP_GEQ:
         return ">=";
      case EXP_LESS:
         return "<";
      case EXP_GRE:
         return ">";
      case EXP_AND:
         return "&&";
      case EXP_OR:
         return "||";
      default:
         return "";
   }
}

static void printNewSize(int n, Exp *e)
{
   if (e == NULL) {
      return;
   }

   printExp(n + 1, e->newexp.size);
}

static void printExp(int n, Exp *e)
{
   if (e == NULL) {
      return;
   }

   if (n > 0) {
      printf(" ");
   }

   printSpaces(n);

   switch (e->tag) {
      case EXP_VAR:
         printRefVar(n, e->varexp.r);
         break;
      case EXP_CALL:
         printCall(n, e->callexp.c);
         break;
      case EXP_INT:
         printf("%d", e->intexp.val);
         break;
      case EXP_FLT:
         printf("%f", e->fltexp.flt);
         break;
      case EXP_STR:
         printf("%s", e->strexp.str);
         break;
      case EXP_ADD...EXP_OR:
         printExp(n, e->binexp.e1);
         printf(" %s", opExp(e->tag));
         printExp(n, e->binexp.e2);
         break;
      case EXP_UNARYMINUS:
         printf("-");
         printExp(n, e->unaryexp.e);
         break;
      case EXP_UNARYNOT:
         printf("!");
         printExp(n, e->unaryexp.e);
         break;
      case EXP_NEW:
         printf("new ");
         printType(e->newexp.t);
         printNewSize(n, e);
         break;
      case EXP_CAST:
         printf("<");
         printExp(n, e->castexp.e);
         printf(" as ");
         printType(e->castexp.t);
         printf(">");
         break;
      case EXP_SEQ:
         printExp(n, e->seqexp.left);
         printExp(n, e->seqexp.right);
         break;
   }
}

static void printStat(int n, Stat *s)
{
   if (s == NULL) {
      return;
   }

   printSpaces(n);

   switch(s->tag) {
      case ST_ATTR:
         printRefVar(n, s->statattr.v);
         printf("=");
         printExp(n + 1, s->statattr.e);
         break;
      case ST_IF:
         printf("if");
         printExp(n, s->statif.e);
         printStat(n, s->statif.ifblock);
         printStat(n, s->statif.elseblock);
         break;
      case ST_WHILE:
         printf("while");
         printExp(n, s->statwhile.e);
         printStat(n, s->statwhile.block);
         break;
      case ST_RETURN:
         printf("return");
         printExp(n, s->statreturn.e);
         break;
      case ST_CALL:
         printCall(n, s->statcall.c);
         break;
      case ST_BLOCK:
         printBlock(n, s->statblock.b);
         break;
      case ST_ECHO:
         printf("echo");
         printExp(n, s->statecho.e);
         break;
      case ST_SEQ:
         printStat(n, s->statseq.left);
         printStat(n, s->statseq.right);
         break;
   }
}
