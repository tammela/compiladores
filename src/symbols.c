#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "util.h"
#include "tree.h"

static lua_State *L;

#define TSYMBOL 1

void *findSymbol(char *id)
{
   void *p;
   lua_Integer len = luaL_len(L, TSYMBOL);

   for (; len > 0; len--) {
      lua_geti(L, TSYMBOL, len);
      if (lua_getfield(L, -1, id) != LUA_TNIL) {
         p = lua_touserdata(L, -1);
         lua_settop(L, TSYMBOL);
         return p;
      }
   }

   lua_settop(L, TSYMBOL);
   return NULL;
}

void addSymbol(Var *v)
{
   lua_Integer len = luaL_len(L, TSYMBOL);

   if (findSymbol(v->id)) {
      perr("%s already declared!", v->id);
      exit(-1);
   }

   lua_geti(L, TSYMBOL, len);
   lua_pushlightuserdata(L, v);
   lua_setfield(L, -2, v->id);
}

void openBlock()
{
   lua_newtable(L);
   lua_seti(L, TSYMBOL, luaL_len(L, TSYMBOL) + 1);
}

void closeBlock()
{
   lua_pushnil(L);
   lua_seti(L, TSYMBOL, luaL_len(L, TSYMBOL));
}

void initSymbolTable()
{
   L = luaL_newstate();

   if (L == NULL) {
      puts(strerror(ENOMEM));
      exit(-1);
   }

   lua_newtable(L);
}
