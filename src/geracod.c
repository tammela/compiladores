#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "util.h"

/* LLVM statement counter */
static int cmdcounter = 0;

/* LLVM label counter */
static int labelcounter = 0;

/* cache */
struct cache {
   int constint;  /* %d */
   int constflt;  /* %f */
   int conststr;  /* %s */
};

static struct cache control;

static int geraVar(RefVar *);
static int getrefIdx(RefVar *);
static int geraExpressoes(Exp *);
static void geraCond(Exp *, int, int, int *);
static const char *bintag2Str(Type *, EXP_tag);
static void percorreStatement(Stat *);

static char *strType(Type *t)
{
   int i;
   char *str = NULL;

   if (!isArray(t)) {
      if (isInt(t) || isChar(t))
         return "i32";
      else if (isFlt(t))
         return "double";
      else if (isVoid(t))
         return "void";
   } else {
      str = malloc(7 + sizeof(char) * arrayDepth(t));
      if(isChar(t))
         strncpy(str, "i8", 3);
      else if (isInt(t))
         strncpy(str, "i32", 4);
      else if(isFlt(t))
         strncpy(str, "double", 7);

      for (i = 0; i < arrayDepth(t); i++)
         strcat(str, "*");

      return str;
   }

   return NULL;
}

static char *stratomType(Type *t)
{
   if (isInt(t) || isChar(t))
      return "i32";
   else if (isFlt(t))
      return "double";
   else if (isVoid(t))
      return "void";

   return NULL;
}


static int getExpLen(Exp *e)
{
   switch (e->tag) {
      case EXP_STR:
         return e->strexp.len;
      default:
         return 0;
   }
}

static int printVarCmd(void)
{
   cmdcounter += 1;
   printf("%%cmd%d", cmdcounter);
   return cmdcounter;
}

static int printCmd(void)
{
   cmdcounter += 1;
   printf("%%cmd%d =", cmdcounter);
   return cmdcounter;
}

static int printGlobalCmd(void)
{
   cmdcounter += 1;
   printf("@cmd%d =", cmdcounter);
   return cmdcounter;
}

static int incCounter(void)
{
   cmdcounter += 1;

   return cmdcounter;
}

static int geraLabel(void)
{
   labelcounter += 1;

   return labelcounter;
}

static void printRet(Var *v)
{
   printf("ret %s\n", strType(v->type));
}

static void printConst(const char *type, int idx, int len, int isglobal)
{
   if (!isglobal) {
      if (len > 0) { /* it is an array! */
         printf("%s* getelementptr ([%d x %s], [%d x %s]* %%cmd%d, i32 0, i32 0)",
               type, len, type, len, type, idx);
      } else {
         printf("%s %%cmd%d", type, idx);
      }
   } else {
      if (len > 0) { /* it is an array! */
         printf("%s* getelementptr ([%d x %s], [%d x %s]* @cmd%d, i32 0, i32 0)",
               type, len, type, len, type, idx);
      } else {
         printf("%s @cmd%d", type, idx);
      }
   }
}

static void printLocals(Def *d)
{
   Var *v;
   int statnum;

   if (d == NULL) {
      return;
   }

   switch (d->tag) {
      case DEF_VAR:
         v = d->vardef.v;
         printf("; Local variable <%s>\n", v->id);
         statnum = printCmd();
         printf(" alloca %s\n", strType(v->type));
         if (v->num != 0) { /* this is a parameter */
            printf("store %%cmd%d, %s* %%cmd%d\n", v->num, strType(v->type), statnum);
         } else {
            if (isArray(v->type)) {
               printf("store %s null, %s* %%cmd%d\n", strType(v->type),
                     strType(v->type), statnum);
            } else {
               if (isFlt(v->type))
                  printf("store double 0.0, double* %%cmd%d\n", statnum);
               else if (isInt(v->type))
                  printf("store i32 0, i32* %%cmd%d\n", statnum);
               else
                  printf("store i8 0, i8* %%cmd%d\n", statnum);
            }
         }
         v->num = statnum;
         printf("; end\n");
         break;
      case DEF_SEQ:
         printLocals(d->seqdef.left);
         printLocals(d->seqdef.right);
         break;
      default:
         assert(0);
   }
}

static int geraEcho(Stat *leaf)
{
   int num, expnum;
   Type *t;
   Exp *e;

   e = leaf->statecho.e;
   t = expType(e);
   expnum = geraExpressoes(e);
   num = printCmd();
   printf(" call i32 (i8 *, ...) @printf(");
   if (isChar(t) && isArray(t)) { /* literal string */
      /* print %s constant */
      printConst("i8", control.conststr, 4, 1);
      printf(",");
      goto out;
   } else if (!isArray(t)) {
      if (isInt(t) || isChar(t)) {
         /* print %d constant */
         printConst("i8", control.constint, 4, 1);
         printf(",");
         goto out;
      } else if (isFlt(t)) {
         /* print %f constant */
         printConst("i8", control.constflt, 6, 1);
         printf(",");
         goto out;
      }
   }

out:
   /* print exp */
   if (e->tag == EXP_STR)
       printConst("i8", expnum, getExpLen(e), 1);
   else
       printConst(strType(t), expnum, getExpLen(e), 0);
   printf(")\n");
   return num;
}

static int getrefIdx(RefVar *r)
{
   switch (r->tag) {
      case REF_VAR:
         return r->refv.v->num;
      case REF_ARRAY:
         return r->refa.num;
   }

   return 0;
}

static void geraAtribuicao(Stat *s)
{
   RefVar *v;
   int expnum, numref;
   Type *type;

   v = s->statattr.v;
   type = expType(s->statattr.e);
   expnum = geraExpressoes(s->statattr.e);

   if (v->tag == REF_ARRAY) {
      geraVar(v);
   }

   numref = getrefIdx(v);

   if (isArray(type) && s->statattr.e->tag != EXP_NEW) {
      printf("store ");
      if (isChar(type)) {
         printConst("i8", expnum, getExpLen(s->statattr.e), 1);
      }
   } else {
      printf("store %s %%cmd%d", strType(type), expnum);
   }

   if (v->tag != REF_ARRAY) {
      if (!v->refv.v->globalnum) {
         printf(", %s* %%cmd%d\n", strType(type), numref);
      } else {
         printf(", %s* @cmd%d\n", strType(type), v->refv.v->globalnum);
      }
   } else {
      printf(", %s* %%cmd%d\n", strType(type), numref);
   }
}

static void geraIf(Stat *s)
{
   int lt = geraLabel();
   int lf = geraLabel();
   int lout = geraLabel();

   printf("; if\n");
   geraCond(s->statif.e, lt, lf, NULL);
   printf("L%d:\n", lt);
   percorreStatement(s->statif.ifblock);
   printf("br label %%L%d\n", lout);
   printf("L%d:\n", lf);
   percorreStatement(s->statif.elseblock);
   printf("br label %%L%d\n", lout);
   printf("L%d:\n", lout);
   printf("; endif\n");
}

static void geraWhile(Stat *s)
{
   int lb = geraLabel();
   int lout = geraLabel();
   int lcmp = 0;

   printf("; while\n");
   geraCond(s->statwhile.e, lb, lout, &lcmp);
   printf("L%d:\n", lb);
   percorreStatement(s->statwhile.block);
   printf("br label %%L%d\n", lcmp);
   printf("L%d:\n", lout);
   printf("; endwhile\n");
}

static void percorreStatement(Stat *s)
{

   if (s == NULL) {
      return;
   }

   switch (s->tag) {
      case ST_BLOCK:
         percorreStatement(s->statblock.b->s);
         break;
      case ST_ATTR:
         geraAtribuicao(s);
         break;
      case ST_IF:
         geraIf(s);
         break;
      case ST_WHILE:
         geraWhile(s);
         break;
      case ST_ECHO:
         geraEcho(s);
         break;
      case ST_SEQ:
         percorreStatement(s->statseq.left);
         percorreStatement(s->statseq.right);
         break;
      default:
         assert(0);
   }
}

static void geraGlobais(Def *d)
{
   int num;
   Type *t;

   if (d == NULL) {
      return;
   }

   switch (d->tag) {
      case DEF_VAR:
         t = d->vardef.v->type;
         num = printGlobalCmd();
         if (!isArray(t)) {
            if (isInt(t) || isChar(t)) {
               printf(" global i32 0\n");
            } else if (isFlt(t)) {
               printf(" global double %e\n", 0.0);
            }
         }
         d->vardef.v->globalnum = num;
         break;
      case DEF_FUNCTION:
         break;
      case DEF_SEQ:
         geraGlobais(d->seqdef.left);
         geraGlobais(d->seqdef.right);
         break;
   }
}

static int geraIndexacao(Exp *idx, int num, char *type)
{
   Exp *p = idx;
   int numgen, numret;
   char *dup;

   if (p->tag != EXP_SEQ) {
      numgen = geraExpressoes(p);
      numret = printCmd();
      dup = strdup(type);
      dup[strlen(dup) - 1] = '\0';
      printf(" getelementptr inbounds %s, %s %%cmd%d, i32 %%cmd%d\n", dup,
            type, num, numgen);
      return numret;
   }

   while (p != NULL) {
      numgen = geraExpressoes(p->seqexp.left);
      numret = printCmd();
      dup = strdup(type);
      dup[strlen(dup) - 1] = '\0';
      printf(" getelementptr inbounds %s, %s %%cmd%d, i32 %%cmd%d\n", dup,
            type, num, numgen);
      type[strlen(type)] = '\0';
      p = p->seqexp.right;
   }

   return numret;
}

static int geraVar(RefVar *r)
{
   Var *v;
   int num;
   int numvar, numidx;
   char *type;

   switch (r->tag) {
      case REF_VAR:
         v = r->refv.v;
         type = strType(v->type);
         num = printCmd();
         if (!v->globalnum) {
            printf(" load %s, %s* %%cmd%d\n", type, type, v->num);
         } else {
            printf(" load %s, %s* @cmd%d\n", type, type, v->globalnum);
         }
         return num;
      case REF_ARRAY:
         numvar = geraExpressoes(r->refa.v);
         type = strType(expType(r->refa.v));
         numidx = geraIndexacao(r->refa.idx, numvar, type);
         r->refa.num = numidx;
         return numidx;
   }

   return 0;
}

static int geraExpressoes(Exp *e)
{
   int nume1, nume2, num;
   int lt, lf, lout;
   RefVar *r;
   Type *t;

   switch (e->tag) {
     case EXP_ADD...EXP_DIV:
        t = expType(e);
        nume1 = geraExpressoes(e->binexp.e1);
        nume2 = geraExpressoes(e->binexp.e2);
        num = printCmd();
        printf(" %s nsw %s %%cmd%d, %%cmd%d\n", bintag2Str(t, e->tag),
              strType(t), nume1, nume2);
        e->binexp.num = num;
        return num;
     case EXP_EQ...EXP_GRE:
         t = expType(e);
         nume1 = geraExpressoes(e->binexp.e1);
         nume2 = geraExpressoes(e->binexp.e2);
         num = printCmd();
         if (!isFlt(t)) {
            printf(" icmp");
         } else {
            printf(" fcmp");
         }
         printf(" %s %s %%cmd%d, %%cmd%d\n", bintag2Str(t, e->tag),
               strType(t), nume1, nume2);
         num = printCmd();
         printf(" zext i1 %%cmd%d to i32\n", num - 1);
         e->binexp.num = num;
         return num;
     case EXP_VAR:
         r = e->varexp.r;
         num = geraVar(r);
         e->varexp.num = num;
         return num;
      case EXP_STR:
         if (e->strexp.num == 0) {
            e->strexp.num = incCounter();
            push2Table(e, 1);
         }
         return e->strexp.num;
      case EXP_INT:
         printf("; Store constant int expression\n");
         num = printCmd();
         printf(" alloca i32\n");
         printf("store i32 %d, i32* %%cmd%d\n", e->intexp.val, num);
         num = printCmd();
         printf(" load i32, i32* %%cmd%d\n", num - 1);
         printf("; end\n");
         e->intexp.num = num;
         return num;
      case EXP_FLT:
         printf("; Store constant float expression\n");
         num = printCmd();
         printf(" alloca double\n");
         printf("store double %e, double* %%cmd%d\n", e->fltexp.flt, num);
         num = printCmd();
         printf(" load double, double* %%cmd%d\n", num - 1);
         printf("; end\n");
         e->fltexp.num = num;
         return num;
      case EXP_CAST:
         t = expType(e->castexp.e);
         nume1 = geraExpressoes(e->castexp.e);
         num = printCmd();
         if (isInt(t) && isFlt(e->castexp.expt)) {
            printf(" sitofp i32 %%cmd%d to double\n", nume1);
         } else if (isChar(t) && isFlt(e->castexp.expt)) {
            printf(" sitofp i8 %%cmd%d to double\n", nume1);
         } else if (isFlt(t) && isInt(e->castexp.expt)) {
            printf(" fptosi double %%cmd%d to i32\n", nume1);
         }
         e->castexp.num = num;
         return num;
      case EXP_AND:
      case EXP_OR:
         lt = geraLabel();
         lf = geraLabel();
         lout = geraLabel();
         geraCond(e, lt, lf, NULL);
         printf("L%d: br label %%L%d\n", lt, lout);
         printf("L%d: br label %%L%d\n", lf, lout);
         printf("L%d:\n", lout);
         num = printCmd();
         printf(" phi i32 [1, %%L%d], [0, %%L%d]\n", lt, lf);
         return num;
      case EXP_NEW:
         nume1 = geraExpressoes(e->newexp.size);
         nume2 = printCmd();
         printf(" call noalias i8* @malloc(i32 %%cmd%d)\n", nume1);
         num = printCmd();
         printf(" bitcast i8* %%cmd%d to %s*\n", nume2, stratomType(expType(e)));
         return num;
      default:
         assert(0);
   }
}

static void printParams(Def *p)
{
   Var *v;
   int num;

   if (p == NULL) {
      return;
   }

   switch (p->tag) {
      case DEF_VAR:
         v = p->vardef.v;
         printf("%s ", strType(v->type));
         num = printVarCmd();
         v->num = num;
         break;
      case DEF_SEQ:
         printParams(p->seqdef.left);
         if (p->seqdef.right != NULL) {
            printf(",");
            printParams(p->seqdef.right);
         }
         break;
      default:
         assert(0);
   }
}

static const char *bintag2Str(Type *t, EXP_tag tag)
{
   switch (tag) {
      case EXP_ADD:
         if (isFlt(t))
            return "fadd";
         return "add";
      case EXP_SUB:
         if (isFlt(t))
            return "fsub";
         return "sub";
      case EXP_MUL:
         if (isFlt(t))
            return "fmul";
         return "mul";
      case EXP_DIV:
         if (isFlt(t))
            return "fdiv";
         return "div";
      case EXP_EQ:
         if (isFlt(t))
            return "oeq";
         return "eq";
      case EXP_NEQ:
         if (isFlt(t))
            return "une";
         return "ne";
      case EXP_LEQ:
         if (isFlt(t))
            return "ole";
         return "sle";
      case EXP_GEQ:
         if (isFlt(t))
            return "oge";
         return "sge";
      case EXP_LESS:
         if (isFlt(t))
            return "olt";
         return "slt";
      case EXP_GRE:
         if (isFlt(t))
            return "ogt";
         return "sgt";
      default:
         assert(0);
   }
}

static void geraCond(Exp *e, int lt, int lf, int *lcmp)
{
   int nume1;
   int nume2;
   int l;
   Type *t;

   switch (e->tag) {
      case EXP_ADD...EXP_DIV:
         t = expType(e);
         nume1 = geraExpressoes(e->binexp.e1);
         nume2 = geraExpressoes(e->binexp.e2);
         printCmd();
         printf(" %s nsw %s %%cmd%d, %%cmd%d\n", bintag2Str(t, e->tag),
               strType(t), nume1, nume2);
         if (lcmp) {
            *lcmp = geraLabel();
            printf("br label %%L%d\n", *lcmp);
            printf("L%d:\n", *lcmp);
         }
         printCmd();
         printf(" icmp ne i32 %%cmd%d, 0\n", cmdcounter - 1);
         goto out;
      case EXP_EQ...EXP_LESS:
         t = expType(e);
         nume1 = geraExpressoes(e->binexp.e1);
         nume2 = geraExpressoes(e->binexp.e2);
         if (lcmp) {
            *lcmp = geraLabel();
            printf("br label %%L%d\n", *lcmp);
            printf("L%d:\n", *lcmp);
         }
         printCmd();
         printf(" icmp %s %s %%cmd%d, %%cmd%d\n", bintag2Str(t, e->tag),
               strType(t), nume1, nume2);
         goto out;
      case EXP_AND:
         l = geraLabel();
         geraCond(e->binexp.e1, l, lf, lcmp);
         printf("L%d:\n", l);
         geraCond(e->binexp.e2, lt, lf, lcmp);
         return;
      case EXP_OR:
         l = geraLabel();
         geraCond(e->binexp.e1, lt, l, lcmp);
         printf("L%d:\n", l);
         geraCond(e->binexp.e2, lt, lf, lcmp);
         return;
      case EXP_VAR:
         if (lcmp) {
            *lcmp = geraLabel();
            printf("br label %%L%d\n", *lcmp);
            printf("L%d:\n", *lcmp);
         }
         nume1 = geraExpressoes(e);
         printCmd();
         printf(" icmp ne i32 %%cmd%d, 0\n", nume1);
         goto out;
      default:
         nume1 = geraExpressoes(e);
         if (lcmp) {
            *lcmp = geraLabel();
            printf("br label %%L%d\n", *lcmp);
            printf("L%d:\n", *lcmp);
         }
         printCmd();
         printf(" icmp ne i32 %%cmd%d, 0\n", nume1);
         goto out;
   }

out:
   printf("br i1 %%cmd%d, label %%L%d, label %%L%d\n", cmdcounter, lt, lf);
}

static void geraFuncoes(Def *f)
{
   if (f == NULL) {
      return;
   }

   switch (f->tag) {
      case DEF_VAR:
         /* globals were generated in phase 1 */
         break;
      case DEF_FUNCTION:
         printf("\n");
         printf("define %s @%s(", strType(f->funcdef.v->type), f->funcdef.v->id);
         printParams(f->funcdef.p);
         printf(") {\n");
         printLocals(f->funcdef.p);
         printLocals(f->funcdef.b->statblock.b->v);
         percorreStatement(f->funcdef.b);
         printRet(f->funcdef.v);
         printf("}\n");
         break;
      case DEF_SEQ:
         geraFuncoes(f->seqdef.left);
         geraFuncoes(f->seqdef.right);
         break;
   }
}

void geraCodigo(Def *root)
{
   prepTable();
   /* generates globals */
   geraGlobais(root);
   /* print all printf format modifiers */
   control.constint = printGlobalCmd();
   printf(" private constant [4 x i8] c\"%%d\\0A\\00\"\n");
   control.constflt = printGlobalCmd();
   printf(" private constant [6 x i8] c\"%%.7f\\0A\\00\"\n");
   control.conststr = printGlobalCmd();
   printf(" private constant [4 x i8] c\"%%s\\0A\\00\"\n");
   printf("declare i32 @printf(i8*, ...)\n");
   printf("declare i8* @malloc(i32)\n");
   /* generates code for each function in the AST tree */
   geraFuncoes(root);
   /* dump constant strings found on the tree */
   dumpTable(1);
   destroyState();
   printf("\n");
}
