#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include "lex.yy.h"
#include "monga.tab.h"

int yydebug = 1;

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

   yyrestart(fdopen(fd, "r"));

   if (yyparse()) {
      fprintf(stdout, "NOT OK");
   } else {
      fprintf(stdout, "OK");
   }

   close(fd);

   return 0;
}
