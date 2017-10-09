#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tree.h"

#include "util.h"

static void *myalloc(size_t n)
{
   void *p;

   p = malloc(n);
   if (p == NULL) {
      perr(strerror(ENOMEM));
   }
   return p;
}

Type *typeNode(MT_tag tag, TYPE_tag typetag)
{
   Type *t = myalloc(sizeof(Type));

   t->atomtype.tag = tag;
   t->atomtype.t = typetag;
   return t;
}

Type *seqtypeNode(MT_tag tag, Type *tr)
{
   Type *t = myalloc(sizeof(Type));

   t->seqtype.tag = tag;
   t->seqtype.right = typeNode(ATOMIC, ARRAY);
   t->seqtype.left = tr;
}

Var *varNode(char *id, Type *type)
{
   Var *v = myalloc(sizeof(Var));

   v->id = id;
   v->type = type;

   return v;
}

Call *callNode(char *id, Exp *e)
{
   Call *c = myalloc(sizeof(Call));

   c->v = varNode(id, NULL);
   c->e = e;

   return c;
}

RefVar *refvNode(Var *v)
{
   RefVar *r = myalloc(sizeof(RefVar));

   r->refv.tag = REF_VAR;
   r->refv.v = v;

   return r;
}

RefVar *refaNode(Exp *v, Exp *idx)
{
   RefVar *r = myalloc(sizeof(RefVar));

   r->refa.tag = REF_ARRAY;
   r->refa.v = v;
   r->refa.idx = idx;

   return r;
}

Def *vardefNode(char *id, Type *type)
{
   Def *d = myalloc(sizeof(Def));

   d->vardef.tag = DEF_VAR;
   d->vardef.v = varNode(id, type);

   return d;
}

Def *funcdefNode(Var *v, Def *p, Stat *b)
{
   Def *d = myalloc(sizeof(Def));

   d->funcdef.tag = DEF_FUNCTION;
   d->funcdef.v = v;
   d->funcdef.b = b;
   d->funcdef.p = p;

   return d;
}

Def *seqdefNode(Def *d1, Def *d2)
{
   Def *d = myalloc(sizeof(Def));

   d->seqdef.tag = DEF_SEQ;
   d->seqdef.left = d1;
   d->seqdef.right = d2;

   return d;
}

Block *blockNode(Def *v, Stat *s)
{
   Block *b = myalloc(sizeof(Block));

   b->s = s;
   b->v = v;

   return b;
}

Stat *blockstatNode(Def *v, Stat *s)
{
   Stat *st = myalloc(sizeof(Stat));

   st->statblock.tag = ST_BLOCK;
   st->statblock.b = blockNode(v, s);

   return st;
}

Stat *ifstatNode(Exp *e, Stat *ifblock, Stat *elseblock)
{
   Stat *s = myalloc(sizeof(Stat));

   s->statif.tag = ST_IF;
   s->statif.e = e;
   s->statif.ifblock = ifblock;
   s->statif.elseblock = elseblock;

   return s;
}

Stat *whilestatNode(Exp *e, Stat *block)
{
   Stat *s = myalloc(sizeof(Stat));

   s->statwhile.tag = ST_WHILE;
   s->statwhile.e = e;
   s->statwhile.block = block;

   return s;
}

Stat *callstatNode(Call *c)
{
   Stat *s = myalloc(sizeof(Stat));

   s->statcall.tag = ST_CALL;
   s->statcall.c = c;

   return s;
}

Stat *retstatNode(Exp *e)
{
   Stat *s = myalloc(sizeof(Exp));

   s->statreturn.tag = ST_RETURN;
   s->statreturn.e = e;

   return s;
}

Stat *echostatNode(Exp *e)
{
   Stat *s = myalloc(sizeof(Exp));

   s->statecho.tag = ST_ECHO;
   s->statecho.e = e;

   return s;
}

Stat *attrstatNode(RefVar *r, Exp *e)
{
   Stat *s = myalloc(sizeof(Stat));

   s->statattr.tag = ST_ATTR;
   s->statattr.e = e;
   s->statattr.v = r;

   return s;
}

Stat *seqstatNode(Stat *s1, Stat *s2)
{
   Stat *s = myalloc(sizeof(Stat));

   s->statseq.tag = ST_SEQ;
   s->statseq.left = s1;
   s->statseq.right = s2;

   return s;
}

Exp *binexpNode(EXP_tag tag, Exp *e1, Exp *e2)
{
   Exp *e = myalloc(sizeof(Exp));

   e->binexp.tag = tag;
   e->binexp.e1 = e1;
   e->binexp.e2 = e2;

   return e;
}

Exp *unaryexpNode(EXP_tag tag, Exp *ep)
{
   Exp *e = myalloc(sizeof(Exp));

   e->unaryexp.tag = tag;
   e->unaryexp.e = ep;

   return e;
}

Exp *castexpNode(Exp *ep, Type *t)
{
   Exp *e = myalloc(sizeof(Exp));

   e->castexp.tag = EXP_CAST;
   e->castexp.t = t;
   e->castexp.e = ep;

   return e;
}

Exp *newexpNode(Exp *ep, Type *t)
{
   Exp *e = myalloc(sizeof(Exp));

   e->newexp.tag = EXP_NEW;
   e->newexp.t = t;
   e->newexp.size = ep;

   return e;
}

Exp *varexpNode(RefVar *r)
{
   Exp *e = myalloc(sizeof(Exp));

   e->varexp.tag = EXP_VAR;
   e->varexp.r = r;

   return e;
}

Exp *callexpNode(Call *c)
{
   Exp *e = myalloc(sizeof(Exp));

   e->callexp.tag = EXP_CALL;
   e->callexp.c = c;

   return e;
}

Exp *intexpNode(int i)
{
   Exp *e = myalloc(sizeof(Exp));

   e->intexp.tag = EXP_INT;
   e->intexp.val = i;
   e->intexp.t = typeNode(ATOMIC, INT);

   return e;
}

Exp *fltexpNode(float f)
{
   Exp *e = myalloc(sizeof(Exp));

   e->fltexp.tag = EXP_FLT;
   e->fltexp.flt = f;
   e->fltexp.t = typeNode(ATOMIC, FLOAT);

   return e;
}

Exp *strexpNode(char *str)
{
   Exp *e = myalloc(sizeof(Exp));

   e->strexp.tag = EXP_STR;
   e->strexp.str = str;
   e->strexp.t = seqtypeNode(SEQ, typeNode(ATOMIC, CHAR));

   return e;
}

Exp *seqexpNode(Exp *e1, Exp *e2)
{
   Exp *e = myalloc(sizeof(Exp));

   e->seqexp.tag = EXP_SEQ;
   e->seqexp.left = e1;
   e->seqexp.right = e2;

   return e;
}
