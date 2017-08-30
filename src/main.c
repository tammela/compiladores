#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tk.h"
#include "lex.yy.h"

const char *tokensname[] = {
   "",
   "int",
   "float",
   "char",
   "void",
   "if",
   "else",
   "while",
   "return",
   "new",
   "as",
   "==",
   "<=",
   ">=",
   "~=",
   "&&",
   "||",
   "@",
   NULL
};

SValue sem;

void scantokens(void)
{
   TK_ENUM token;

   while ((token = yylex()) != 0) {
      switch (token) {
         case TK_INT:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_FLOAT:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_CHAR:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_VOID:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_IF ... TK_OR:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_ID:
            puts(sem.str);
            free(sem.str);
            break;
         case TK_PRINT:
            puts(tokensname[token - CHAR_MAX]);
            break;
         case TK_STRING:
            puts(sem.str);
            free(sem.str);
            break;
         case TK_SINT:
            printf("%d\n", sem.inum);
            break;
         case TK_SFLOAT:
            printf("%g\n", sem.fnum);
            break;
         case TK_LITERAL:
            putchar(sem.str[0]);
            putchar('\n');
            break;
         default:
            putchar(token);
            putchar('\n');
            break;
      }
   }
}


int main(int argc, char **argv)
{
   int fd;

   if (argc < 2) {
      puts("No input");
      return (-1);
   }

   if ((fd = open(argv[1], O_RDONLY)) == -1) {
      puts(strerror(errno));
      return (-1);
   }

   yyout = fdopen(2, "w");
   yyrestart(fdopen(fd, "r"));

   scantokens();
   close(fd);

   return 0;
}
