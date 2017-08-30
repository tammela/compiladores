/* lex para monga */

%{
#include <string.h>
#include "tk.h"

char *copyid();
char *copys();
char *copyl();
int copyint();
int copyhint();
float copyflt();
float copyhflt();
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

@                               { return TK_PRINT; }

{id}                            {
                                    sem.str = copyid();
                                    return TK_ID;
                                }

{string}                        {
                                    sem.str = copys();
                                    return TK_STRING;
                                }

{literal}                       {
                                    sem.str = copyl();
                                    return TK_LITERAL;
                                }
{decflt}                        {
                                    sem.fnum = copyflt();
                                    return TK_SFLOAT;
                                }

{hexflt}                        {
                                    sem.fnum = copyhflt();
                                    return TK_SFLOAT;
                                }

{decint}                        {
                                    sem.inum = copyint();
                                    return TK_SINT;
                                }

{hexint}                        {
                                    sem.inum = copyhint();
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

char *copyid()
{
   char *str = copyalloc(yyleng + 1);

   strncpy(str, yytext, yyleng + 1);

   return str;
}

char escape(char c)
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

char *copys()
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

char *copyl()
{
   char *str = copyalloc(1);

   if (yyleng == 4)
      str[0] = escape(yytext[2]);
   else
      str[0] = yytext[1];

   return str;
}

int copyint()
{
   return strtol(yytext, NULL, 10);
}

int copyhint()
{
   return strtol(yytext, NULL, 16);
}

float copyflt()
{
   return strtod(yytext, NULL);
}

float copyhflt()
{
   return strtod(yytext, NULL);
}
