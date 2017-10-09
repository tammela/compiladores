#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "lex.yy.h"
#include "tree.h"
#include "monga.tab.h"
#include "analyze.h"
#include "geracod.h"

/* set to true if you want debug info */
int yydebug = 0;

Def *AST_tree;

int main(int argc, char **argv)
{
   int fd, ret;

   AST_tree = malloc(sizeof(Def));

   if (AST_tree == NULL) {
      puts(strerror(ENOMEM));
      exit(-1);
   }

   if (argc < 2) {
      puts("No input");
      return -1;
   }

   if ((fd = open(argv[1], O_RDONLY)) == -1) {
      puts(strerror(errno));
      return -1;
   }

   yyrestart(fdopen(fd, "r"));

   if (yyparse()) {
      exit(-1);
   }

   analyzeTree(AST_tree);
   printTree(AST_tree);
   printf("\n");
   close(1);

   if ((ret = open("out.ll", O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
      fprintf(stderr, "%s", strerror(errno));
      return -1;
   }

   geraCodigo(AST_tree);

   fsync(1);
   close(1);
   close(fd);

   return 0;
}
