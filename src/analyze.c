#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "symbols.h"
#include "util.h"

static Def *funcdefs;
static Def *lastfuncdef;
static int isfirst = 1;

Type *expType(Exp *);
void analyzeFunctionDef(Def *);
void analyzeStatement(Stat *);
void analyzeBlockStatement(Stat *);
void analyzeVariable(RefVar *);
void analyzeExp(Exp *);

void fillScope(Def *root)
{
   if (root == NULL) {
      return;
   }

   switch (root->tag) {
      case DEF_SEQ:
         if (root->seqdef.left != NULL) {
            if (root->seqdef.left->tag == DEF_FUNCTION && isfirst) {
               funcdefs = root;
               isfirst = 0;
            }
         }
         fillScope(root->seqdef.left);
         fillScope(root->seqdef.right);
         break;
      case DEF_VAR:
         addSymbol(root->vardef.v);
         break;
      case DEF_FUNCTION:
         addSymbol(root->funcdef.v);
         break;
   }
}

int isVoid(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == VOID;
      case SEQ:
         return isVoid(t->seqtype.right);
   }
}


int isFlt(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == FLOAT;
      case SEQ:
         return isFlt(t->seqtype.right);
   }
}

int isInt(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == INT;
      case SEQ:
         return isInt(t->seqtype.right);
   }
}

int isChar(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == CHAR;
      case SEQ:
         return isChar(t->seqtype.right);
   }
}

int compareType(Type *t1, Type *t2)
{
   if (t1->tag != t2->tag) {
      return 0;
   }

   if (t1->tag == SEQ) {
      return compareType(t1->seqtype.right, t2->seqtype.right);
   }

   return (t1->atomtype.t == t2->atomtype.t);
}

Type *getrefType(RefVar *r)
{
   switch (r->tag) {
      case REF_VAR:
         return r->refv.v->type;
      case REF_ARRAY:
         return expType(r->refa.v);
   }
}

Type *expType(Exp *e)
{
   switch (e->tag) {
      case EXP_ADD...EXP_OR:
         return e->binexp.t;
      case EXP_CALL:
         return e->callexp.c->v->type;
      case EXP_UNARYMINUS:
      case EXP_UNARYNOT:
         return e->unaryexp.t;
      case EXP_VAR:
         return getrefType(e->varexp.r);
      case EXP_INT:
         return e->intexp.t;
      case EXP_FLT:
         return e->fltexp.t;
      case EXP_STR:
         return e->strexp.t;
      case EXP_CAST:
         return e->castexp.t;
      default:
         break;
   }
}

void analyzeTree(Def *root)
{
   if (root == NULL) {
      return;
   }

   initSymbolTable();
   /* Global Scope */
   openBlock();
   fillScope(root);
   analyzeFunctionDef(funcdefs);
   closeBlock();
}

void analyzeFunctionDef(Def *f)
{
   if (f == NULL) {
      return;
   }

   switch (f->tag) {
      case DEF_SEQ:
         analyzeFunctionDef(f->seqdef.left);
         analyzeFunctionDef(f->seqdef.right);
         break;
      case DEF_FUNCTION:
         lastfuncdef = f;
         openBlock();
         fillScope(f->funcdef.p);
         analyzeBlockStatement(f->funcdef.b);
         closeBlock();
         break;
      default:
         break;
   }
}

void analyzeStatement(Stat *s)
{
   Type *et;
   Type *vt;
   Type *ct;

   if (s == NULL) {
      return;
   }

   switch (s->tag) {
      case ST_ATTR:
         analyzeVariable(s->statattr.v);
         analyzeExp(s->statattr.e);
         et = expType(s->statattr.e);
         vt = getrefType(s->statattr.v);
         if (!compareType(vt, et)) {
            if (!isChar(vt) && isInt(et) || !isInt(vt)  && isChar(et)) {
               s->statattr.e = castexpNode(s->statattr.e, vt);
            }
         }
         break;
      case ST_IF:
         analyzeExp(s->statif.e);
         et = expType(s->statif.e);
         if (isVoid(et)) {
            perr("statement is expectating a non-void type");
         }
         if (!isInt(et)) {
            s->statif.e = castexpNode(s->statif.e, typeNode(ATOMIC, INT));
         }
         analyzeBlockStatement(s->statif.ifblock);
         analyzeBlockStatement(s->statif.elseblock);
         break;
      case ST_WHILE:
         analyzeExp(s->statwhile.e);
         et = expType(s->statwhile.e);
         if (isVoid(et)) {
            perr("statement is expectating a non-void type");
         }
         if (!isInt(et)) {
            s->statwhile.e = castexpNode(s->statwhile.e, typeNode(ATOMIC, INT));
         }
         break;
      case ST_RETURN:
         if (s->statreturn.e != NULL) {
            analyzeExp(s->statreturn.e);
            if (!compareType(expType(s->statreturn.e), lastfuncdef->funcdef.v->type)) {
               s->statreturn.e = castexpNode(s->statreturn.e,
                     lastfuncdef->funcdef.v->type);
            }
         }
         break;
      case ST_CALL:
         if ((ct = findSymbol(s->statcall.c->v->id)) == NULL) {
            perr("%s is not declared!!", s->statcall.c->v->id);
         }
         analyzeExp(s->statcall.c->e);
         break;
      case ST_BLOCK:
         analyzeBlockStatement(s);
         break;
      case ST_ECHO:
         analyzeExp(s->statecho.e);
         break;
      case ST_SEQ:
         analyzeStatement(s->statseq.left);
         analyzeStatement(s->statseq.right);
         break;
   }
}

void analyzeBlockStatement(Stat *s)
{
   if (s == NULL) {
      return;
   }

   openBlock();
   fillScope(s->statblock.b->v);
   analyzeStatement(s->statblock.b->s);
   closeBlock();
}

void analyzeVariable(RefVar *r)
{
   Var *v;
   Type *idxtype;

   switch (r->tag) {
      case REF_VAR:
         if ((v = findSymbol(r->refv.v->id)) == NULL) {
            perr("%s is not declared!!", r->refv.v->id);
         }
         if (r->refv.v->type == NULL) {
            r->refv.v->type = v->type;
         }
         break;
      case REF_ARRAY:
         analyzeExp(r->refa.v);
         analyzeExp(r->refa.idx);
         idxtype = expType(r->refa.idx);
         if (!compareType(idxtype, typeNode(ATOMIC, INT))) {
            perr("array indexes must be integers!");
         }
         break;
   }
}

void analyzeExp(Exp *e)
{
   Var *v;
   Type *te1;
   Type *te2;

   if (e == NULL) {
      return;
   }

   switch (e->tag) {
      case EXP_ADD...EXP_GRE:
         analyzeExp(e->binexp.e1);
         analyzeExp(e->binexp.e2);
         te1 = expType(e->binexp.e1);
         te2 = expType(e->binexp.e2);
         if (!compareType(te1, te2)) {
            if (isFlt(te1) && isInt(te2)) {
               e->binexp.e2 = castexpNode(e->binexp.e2, typeNode(ATOMIC, FLOAT));
               e->binexp.t = typeNode(ATOMIC, FLOAT);
            } else if (isInt(te1) && isFlt(te2)) {
               e->binexp.e1 = castexpNode(e->binexp.e1, typeNode(ATOMIC, FLOAT));
               e->binexp.t = typeNode(ATOMIC, FLOAT);
            } else if (isChar(te1) && isInt(te2)) {
               e->binexp.t = typeNode(ATOMIC, INT);
            } else if (isInt(te1) && isChar(te2)) {
               e->binexp.t = typeNode(ATOMIC, INT);
            } else if (isChar(te1) && isFlt(te2)) {
               perr("binary operation with float and char!!");
            } else if (isFlt(te2) && isChar(te2)) {
               perr("binary operation with float and char!!");
            }
         } else {
            e->binexp.t = te1;
         }
         break;
      case EXP_CALL:
         if ((v = findSymbol(e->callexp.c->v->id)) == NULL) {
            perr("%s is not declared!!", e->callexp.c->v->id);
         }
         if (e->callexp.c->v->type == NULL) {
            e->callexp.c->v->type = v->type;
         }
         analyzeExp(e->callexp.c->e);
         break;
      case EXP_UNARYMINUS:
         analyzeExp(e->unaryexp.e);
         e->unaryexp.t = expType(e->unaryexp.e);
         break;
      case EXP_UNARYNOT:
         analyzeExp(e->unaryexp.e);
         e->unaryexp.t = typeNode(ATOMIC, INT);
         break;
      case EXP_VAR:
         analyzeVariable(e->varexp.r);
         break;
      case EXP_AND:
      case EXP_OR:
         analyzeExp(e->binexp.e1);
         analyzeExp(e->binexp.e2);
         te1 = expType(e->binexp.e1);
         te2 = expType(e->binexp.e2);
         if (!isInt(te1)) {
            e->binexp.e1 = castexpNode(e->binexp.e1, typeNode(ATOMIC, INT));
         }
         if (!isInt(te2)) {
            e->binexp.e2 = castexpNode(e->binexp.e2, typeNode(ATOMIC, INT));
         }
         e->binexp.t = typeNode(ATOMIC, INT);
         break;
      case EXP_INT:
         e->intexp.t = typeNode(ATOMIC, INT);
         break;
      case EXP_FLT:
         e->fltexp.t = typeNode(ATOMIC, FLOAT);
         break;
      case EXP_STR:
         e->strexp.t = seqtypeNode(SEQ, typeNode(ATOMIC, CHAR));
         break;
      case EXP_SEQ:
         analyzeExp(e->seqexp.left);
         analyzeExp(e->seqexp.right);
         break;
      default:
         break;
   }
}
