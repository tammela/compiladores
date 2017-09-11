%{
#include <string.h>
#include "monga.tab.h"

static char *copyid();
static char *copys();
static char *copyl();
static int copyint();
static int copyhint();
static float copyflt();
static float copyhflt();
%}

comment         \#[^\n]*\n

id              [a-zA-Z_][a-zA-Z0-9_]*
string          \"([^\\"\n]|\\.)*\"
literal         \'(.|\\n|\\t|\\\\)\'

decint          [0-9]+
hexint          0[xX][0-9a-fA-F]+

expoent         ([EepP][-+]?[0-9]+)?
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
                                    yylval.str = copyid();
                                    return TK_ID;
                                }

{string}                        {
                                    yylval.str = copys();
                                    return TK_STRING;
                                }

{literal}                       {
                                    yylval.str = copyl();
                                    return TK_LITERAL;
                                }
{decflt}                        {
                                    yylval.fnum = copyflt();
                                    return TK_SFLOAT;
                                }

{hexflt}                        {
                                    yylval.fnum = copyhflt();
                                    return TK_SFLOAT;
                                }

{decint}                        {
                                    yylval.inum = copyint();
                                    return TK_SINT;
                                }

{hexint}                        {
                                    yylval.inum = copyhint();
                                    return TK_SINT;
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
      puts(strerror(ENOMEM));
      exit(-1);
   }

   return ptr;
}

static char *copyid()
{
   char *str = copyalloc(yyleng + 1);

   strncpy(str, yytext, yyleng + 1);

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
      default:
         return '\0';
         break;
   }
}

static char *copys()
{
   int i = 0;
   char *str = copyalloc(yyleng - 1);
   char *p = yytext;

   while (*p) {
      switch (*p) {
         case '\\':
            str[i] = escape(*(p + 1));
            p += 2;
            break;
         case '"':
            p++;
            i--;
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

static char *copyl()
{
   char *str = copyalloc(1);

   if (yyleng == 4) {
      str[0] = escape(yytext[2]);
   } else {
      str[0] = yytext[1];
   }

   return str;
}

static int copyint()
{
   return strtol(yytext, NULL, 10);
}

static int copyhint()
{
   return strtol(yytext, NULL, 16);
}

static float copyflt()
{
   return strtod(yytext, NULL);
}

static float copyhflt()
{
   return strtod(yytext, NULL);
}
