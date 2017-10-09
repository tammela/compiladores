%{
#include <string.h>
#include "monga.tab.h"

static char *copyid(char *, size_t);
static char *copys(char *, size_t);
static int copyl(char *);
static int copyint(char *);
static int copyhint(char *);
static float copyflt(char *);
static float copyhflt(char *);

#define perr(msg)  { fprintf(stderr, "%s ", msg); exit(-1); }

%}

comment         \#[^\n]*

id              [a-zA-Z_][a-zA-Z0-9_]*
string          \"([^\\"\n]|\\.)*\"
literal         \'([^\']|\\n|\\t|\\\\|\\\')\'

decint          [0-9]+
hexint          0[xX][0-9a-fA-F]+

expoent         ([pP][-+]?[0-9]+)?
decflt          {decint}\.{decint}
hexflt          {hexint}\.[0-9a-fA-F]+{expoent}

%%

int                             { return TK_INT; }

float                           { return TK_FLOAT; }

char                            { return TK_CHAR; }

void                            { return TK_VOID; }

if                              { return TK_IF; }

else                            { return TK_ELSE; }

while                           { return TK_WHILE; }

return                          { return TK_RETURN; }

new                             { return TK_NEW; }

as                              { return TK_AS; }

{id}                            {
                                    yylval.str = copyid(yytext, yyleng);
                                    return TK_ID;
                                }

{string}                        {
                                    yylval.str = copys(yytext, yyleng);
                                    return TK_STRING;
                                }

{literal}                       {
                                    yylval.inum = copyl(yytext);
                                    return TK_VINT;
                                }
{decflt}                        {
                                    yylval.fnum = copyflt(yytext);
                                    return TK_VFLOAT;
                                }

{hexflt}                        {
                                    yylval.fnum = copyhflt(yytext);
                                    return TK_VFLOAT;
                                }

{decint}                        {
                                    yylval.inum = copyint(yytext);
                                    return TK_VINT;
                                }

{hexint}                        {
                                    yylval.inum = copyhint(yytext);
                                    return TK_VINT;
                                }

{comment}                       /* noop */

"&&"                            { return TK_AND; }

"||"                            { return TK_OR; }

">="                            { return TK_GEQUALS; }

"<="                            { return TK_LEQUALS; }

"=="                            { return TK_EQUALS; }

"~="                            { return TK_NEQUALS; }

[ \t\n]+                        /* noop */

.                               { return yytext[0]; }

%%

static void *copyalloc(size_t arraysz)
{
   void *ptr = calloc(arraysz, sizeof(char));

   if (ptr == NULL) {
      perr(strerror(ENOMEM));
   }

   return ptr;
}

static char *copyid(char *ptr, size_t len)
{
   char *str = copyalloc(len + 1);

   strncpy(str, ptr, len + 1);

   return str;
}

static char escape(char c)
{
   switch(c) {
      case 'n':
         return '\n';
         break;
      case 't':
         return '\t';
         break;
      case '\\':
         return '\\';
         break;
      case '"':
         return '"';
         break;
      case '\'':
         return '\'';
         break;
      default:
         perr("Invalid escape sequence");
         break;
   }
}

static char *copys(char *ptr, size_t len)
{
   int i = 0;
   char *str = copyalloc(len - 1);
   char *p = ptr;

   while (*p != '"') {
      switch (*p) {
         case '\\':
            str[i] = escape(*(p + 1));
            p += 2;
            break;
         default:
            str[i] = *p;
            p++;
            break;
      }
      i++;
   }

   str[i + 1] = '\0';

   return str;
}

static int copyl(char *ptr)
{
   int val;

   if (ptr[1] == '\\') {
      val = escape(ptr[2]);
   } else {
      val = ptr[1];
   }
   return val;
}

static int copyint(char *ptr)
{
   return strtol(ptr, NULL, 10);
}

static int copyhint(char *ptr)
{
   return strtol(ptr, NULL, 16);
}

static float copyflt(char *ptr)
{
   return strtod(ptr, NULL);
}

static float copyhflt(char *ptr)
{
   return strtod(ptr, NULL);
}
