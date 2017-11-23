#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "tree.h"
#include "util.h"

static lua_State *L = NULL;

int isVoid(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == VOID;
      case SEQ:
         return isVoid(t->seqtype.right);
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}

int isFlt(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == FLOAT;
      case SEQ:
         return isFlt(t->seqtype.right);
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}

int isInt(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == INT;
      case SEQ:
         return isInt(t->seqtype.right);
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}

int isChar(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return t->atomtype.t == CHAR;
      case SEQ:
         return isChar(t->seqtype.right);
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}

int isArray(Type *t)
{
   switch (t->tag) {
      case ATOMIC:
         return 0;
      case SEQ:
         return 1;
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}

Type *expType(Exp *e)
{
   switch (e->tag) {
      case EXP_ADD...EXP_OR:
         return e->binexp.expt;
      case EXP_CALL:
         return e->callexp.c->v->type;
      case EXP_UNARYMINUS:
      case EXP_UNARYNOT:
         return e->unaryexp.expt;
      case EXP_VAR:
         return getrefType(e->varexp.r);
      case EXP_INT:
         return e->intexp.expt;
      case EXP_FLT:
         return e->fltexp.expt;
      case EXP_STR:
         return e->strexp.expt;
      case EXP_CAST:
         return e->castexp.expt;
      case EXP_NEW:
         return e->newexp.expt;
      default:
         return NULL;
   }
}

Type *getrefType(RefVar *r)
{
   switch (r->tag) {
      case REF_VAR:
         return r->refv.v->type;
      case REF_ARRAY:
         return expType(r->refa.v);
      default:
         perr("%s : unknown refvar tag!", __FUNCTION__);
         return 0;
   }
}

void *allocstring(size_t sz)
{
   void *p;

   p = calloc(sz, sizeof(char));

   if (p == NULL) {
      perr("no memory");
   }

   return p;
}

void prepTable(void)
{
   if (!L) {
      L = luaL_newstate();

      if (L == NULL) {
         perr("no memory");
      }
   }

   lua_newtable(L);
}

int push2Table(void *p, int tableidx)
{
   size_t idx = luaL_len(L, tableidx);

   if (p == NULL) {
      return 0;
   }

   lua_pushlightuserdata(L, p);
   lua_seti(L, tableidx, idx + 1);

   return idx + 1;
}

void dumpTable(int tableidx)
{
   Exp *e;
   size_t idx = luaL_len(L, tableidx);

   for (; idx > 0; idx--) {
      lua_geti(L, tableidx, idx);
      e = lua_touserdata(L, -1);
      switch (e->tag) {
         case EXP_STR:
            printf("@cmd%d = private unnamed_addr constant [%d x i8] c\"%s\"\n",
                  e->strexp.num, e->strexp.len, e->strexp.str);
            break;
         default:
            perr("%s : tag not implemented!", __FUNCTION__);
      }
   }

   lua_settop(L, tableidx - 1);
}

void destroyState(void)
{
   lua_close(L);
}

int arrayDepth(Type *t)
{
   if (t == NULL) {
      return 0;
   }

   switch (t->tag) {
      case ATOMIC:
         return 0;
      case SEQ:
         return 1 + arrayDepth(t->seqtype.right);
      default:
         perr("%s : unknown type tag!", __FUNCTION__);
         return 0;
   }
}
