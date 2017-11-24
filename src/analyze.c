#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tree.h"
#include "symbols.h"
#include "util.h"

static Def *funcdefs;
static Def *lastfuncdef;
static int isfirst = 1;

void analyzeTree(Def *);

static void fillScope(Def *);
static int compareType(Type *, Type *);
static void analyzeFunctionDef(Def *);
static void analyzeStatement(Stat *);
static void analyzeBlockStatement(Stat *);
static void analyzeVariable(RefVar *);
static void analyzeExp(Exp *);


static int compareType(Type *t1, Type *t2)
{
   if (t1->tag != t2->tag) {
      return 0;
   }

   if (t1->tag == SEQ) {
      return compareType(t1->seqtype.right, t2->seqtype.right);
   }

   return t1->atomtype.t == t2->atomtype.t;
}

static void fillScope(Def *root)
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

static void analyzeFunctionDef(Def *f)
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

static void analyzeStatement(Stat *s)
{
   Type *et;
   Type *vt;
   Type *ct;
   Type *ft;

   if (s == NULL) {
      return;
   }

   switch (s->tag) {
      case ST_ATTR:
         analyzeVariable(s->statattr.v);
         analyzeExp(s->statattr.e);
         et = expType(s->statattr.e);
         vt = getrefType(s->statattr.v);
         if (isVoid(et)) {
            perr("%s : variable is expecting a non-void attribution!", FUNC_ID(lastfuncdef));
         } else if (!compareType(vt, et)) {
            if (isFlt(vt) && isInt(et)) {
               s->statattr.e = castexpNode(s->statattr.e, typeNode(ATOMIC, FLOAT));
            } else if (isInt(vt) && isFlt(et)) {
               s->statattr.e = castexpNode(s->statattr.e, typeNode(ATOMIC, INT));
            }
            else if ((!isChar(vt) && isInt(et)) || (!isInt(vt)  && isChar(et))) {
               s->statattr.e = castexpNode(s->statattr.e, vt);
            }
         }
         break;
      case ST_IF:
         analyzeExp(s->statif.e);
         et = expType(s->statif.e);
         if (s->statif.e->tag == EXP_STR) {
            perr("%s : literal string as a condition!", FUNC_ID(lastfuncdef))
         }
         if (isVoid(et)) {
            perr("%s : statement is expecting a non-void type!", FUNC_ID(lastfuncdef));
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
         if (s->statwhile.e->tag == EXP_STR) {
            perr("%s : literal string as a condition!", FUNC_ID(lastfuncdef))
         }
         if (isVoid(et)) {
            perr("%s : statement is expectating a non-void type!", FUNC_ID(lastfuncdef));
         }
         if (!isInt(et)) {
            s->statwhile.e = castexpNode(s->statwhile.e,
                  typeNode(ATOMIC, INT));
         }
         analyzeBlockStatement(s->statwhile.block);
         break;
      case ST_RETURN:
         if (s->statreturn.e != NULL) {
            analyzeExp(s->statreturn.e);
            ft = lastfuncdef->funcdef.v->type;
            et = expType(s->statreturn.e);
            if (!compareType(et, ft)) {
               if ((!isArray(et) && isArray(ft)) ||
                        (isArray(et) && !isArray(ft))) {
                  perr("incompatible return from function %s",
                        lastfuncdef->funcdef.v->id);
               } else if (arrayDepth(et) != arrayDepth(ft)) {
                  perr("returning array depth does not match in function %s!",
                        lastfuncdef->funcdef.v->id);
               }
               s->statreturn.e = castexpNode(s->statreturn.e,
                     lastfuncdef->funcdef.v->type);
            }
         }
         break;
      case ST_CALL:
         if ((ct = findSymbol(s->statcall.c->v->id)) == NULL) {
            perr("%s : function %s is not declared!!", FUNC_ID(lastfuncdef),
                  s->statcall.c->v->id);
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

static void analyzeBlockStatement(Stat *s)
{
   if (s == NULL) {
      return;
   }

   openBlock();
   fillScope(s->statblock.b->v);
   analyzeStatement(s->statblock.b->s);
   closeBlock();
}

static void validateArray(Exp *e)
{
   if (e->tag != EXP_VAR && e->tag != EXP_CALL) {
      perr("%s : forbidden index access, expression is not an array",
            FUNC_ID(lastfuncdef));
   }
}

static void analyzeVariable(RefVar *r)
{
   Var *v;
   Type *idxtype;

   switch (r->tag) {
      case REF_VAR:
         if ((v = findSymbol(r->refv.v->id)) == NULL) {
            perr("%s : variable %s is not declared!!", FUNC_ID(lastfuncdef),
                  r->refv.v->id);
         }
         if (r->refv.v->type == NULL) {
            free(r->refv.v);
            r->refv.v = v;
         }
         break;
      case REF_ARRAY:
         validateArray(r->refa.v);
         analyzeExp(r->refa.v);
         analyzeExp(r->refa.idx);
         idxtype = expType(r->refa.idx);
         if (!compareType(idxtype, typeNode(ATOMIC, INT))) {
            r->refa.idx = castexpNode(r->refa.idx, typeNode(ATOMIC, INT));
         }
         break;
   }
}

static int isIndexed(Exp *e)
{
   return e->tag == EXP_VAR && e->varexp.r->tag == REF_ARRAY;
}

static void fixexpTypes(Exp *e, Type *te1, Type *te2)
{
   switch (e->tag) {
      case EXP_ADD...EXP_GRE:
         /* adjust indexed expressions */
         if (isIndexed(e->binexp.e1)) {
            e->binexp.e1 = castexpNode(e->binexp.e1,
                  expType(e->binexp.e2));
            te1 = te2;
            e->binexp.expt = te1;
         } else if (isIndexed(e->binexp.e2)) {
            e->binexp.e2 = castexpNode(e->binexp.e2,
                  expType(e->binexp.e1));
            te2 = te1;
            e->binexp.expt = te2;
         } else {
            /* adjust binary operation types */
            if (isFlt(te1) && isInt(te2)) {
               e->binexp.e2 = castexpNode(e->binexp.e2,
                     typeNode(ATOMIC, FLOAT));
               e->binexp.expt = typeNode(ATOMIC, FLOAT);
            } else if (isInt(te1) && isFlt(te2)) {
               e->binexp.e1 = castexpNode(e->binexp.e1,
                     typeNode(ATOMIC, FLOAT));
               e->binexp.expt = typeNode(ATOMIC, FLOAT);
            } else if (isChar(te1) && isInt(te2)) {
               e->binexp.expt = typeNode(ATOMIC, INT);
            } else if (isInt(te1) && isChar(te2)) {
               e->binexp.expt = typeNode(ATOMIC, INT);
            } else if (isChar(te1) && isFlt(te2)) {
               e->binexp.e1 = castexpNode(e->binexp.e1,
                     typeNode(ATOMIC, FLOAT));
               e->binexp.expt = typeNode(ATOMIC, FLOAT);
            } else if (isFlt(te1) && isChar(te2)) {
               e->binexp.e2 = castexpNode(e->binexp.e2,
                     typeNode(ATOMIC, FLOAT));
               e->binexp.expt = typeNode(ATOMIC, FLOAT);
            }
         }
         break;
      default:
         perr("unknown tag!");
         break;
   }
}

static void analyzeExp(Exp *e)
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
         if (!compareType(te1, te2)) { /* types don't match */
            if ((isArray(te1) && !isIndexed(e->binexp.e1)) ||
                  (isArray(te2) && !isIndexed(e->binexp.e2))) {
               perr("%s : array index is missing", FUNC_ID(lastfuncdef));
            }
            fixexpTypes(e, te1, te2);
         } else { /* types match */
            e->binexp.expt = te1;
         }
         break;
      case EXP_CALL:
         if ((v = findSymbol(e->callexp.c->v->id)) == NULL) {
            perr("%s : %s is not declared!!", FUNC_ID(lastfuncdef), e->callexp.c->v->id);
         }
         if (e->callexp.c->v->type == NULL) {
            free(e->callexp.c->v);
            e->callexp.c->v = v;
         }
         analyzeExp(e->callexp.c->e);
         break;
      case EXP_UNARYMINUS:
         analyzeExp(e->unaryexp.e);
         e->unaryexp.expt = expType(e->unaryexp.e);
         break;
      case EXP_UNARYNOT:
         analyzeExp(e->unaryexp.e);
         e->unaryexp.expt = typeNode(ATOMIC, INT);
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
         e->binexp.expt = typeNode(ATOMIC, INT);
         break;
      case EXP_NEW:
         analyzeExp(e->newexp.size);
         if (!isInt(expType(e->newexp.size))) {
            e->newexp.size = castexpNode(e->newexp.size,
                  typeNode(ATOMIC, INT));
         }
         break;
      case EXP_SEQ:
         analyzeExp(e->seqexp.left);
         analyzeExp(e->seqexp.right);
         break;
      default:
         break;
   }
}
