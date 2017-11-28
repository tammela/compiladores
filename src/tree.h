#ifndef _TREE_H
#define _TREE_H
typedef union mongastat Stat;
typedef union type Type;
typedef union exp Exp;
typedef union def Def;
typedef union refvar RefVar;

typedef struct call Call;
typedef struct var Var;
typedef struct block Block;

struct var {
   int num;
   int globalnum;
   char *id;
   Type *type;
};

struct call {
   Var *v;
   Exp *e;
};

struct block {
   Def *v;
   Stat *s;
};

typedef enum {
   REF_VAR,
   REF_ARRAY
} RV_tag;

union refvar {
   RV_tag tag;
   struct {
      RV_tag tag;
      Var *v;
   } refv;
   struct {
      RV_tag tag;
      int num;
      Exp *v;
      Exp *idx;
   } refa;
};

/* type tags */
typedef enum {
   INT,
   CHAR,
   VOID,
   FLOAT
} TYPE_tag;

typedef enum {
   ATOMIC,
   SEQ
} MT_tag;

union type {
   MT_tag tag;
   struct {
      MT_tag tag;
      TYPE_tag t;
   } atomtype;
   struct {
      MT_tag tag;
      Type *left;
      Type *right;
   } seqtype;
};

/* definition tags */
typedef enum {
   DEF_VAR,
   DEF_FUNCTION,
   DEF_SEQ
} DEF_tag;

union def {
   DEF_tag tag;
   struct {
      DEF_tag tag;
      Var *v;
   } vardef;
   struct {
      DEF_tag tag;
      Var *v;
      Def *p;
      Stat *b;
   } funcdef;
   struct {
      DEF_tag tag;
      Def *left;
      Def *right;
   } seqdef;
};

/* statement tags */
typedef enum {
   ST_ATTR,
   ST_IF,
   ST_WHILE,
   ST_RETURN,
   ST_CALL,
   ST_BLOCK,
   ST_ECHO,
   ST_SEQ
} ST_tag;

union mongastat {
   ST_tag tag;
   struct {
      ST_tag tag;
      RefVar *v;
      Exp *e;
   } statattr;
   struct {
      ST_tag tag;
      Exp *e;
      Stat *ifblock;
      Stat *elseblock;
   } statif;
   struct {
      ST_tag tag;
      Exp *e;
      Stat *block;
   } statwhile;
   struct {
      ST_tag tag;
      Exp *e;
   } statreturn;
   struct {
      ST_tag tag;
      Call *c;
   } statcall;
   struct {
      ST_tag tag;
      Exp *e;
   } statecho;
   struct {
      ST_tag tag;
      Block *b;
   } statblock;
   struct {
      ST_tag tag;
      Stat *left;
      Stat *right;
   } statseq;
};

/* expression tags */
typedef enum {
   EXP_VAR,
   EXP_CALL,
   EXP_INT,
   EXP_FLT,
   EXP_STR,
   EXP_ADD,
   EXP_SUB,
   EXP_MUL,
   EXP_DIV,
   EXP_EQ,
   EXP_NEQ,
   EXP_LEQ,
   EXP_GEQ,
   EXP_LESS,
   EXP_GRE,
   EXP_AND,
   EXP_OR,
   EXP_UNARYMINUS,
   EXP_UNARYNOT,
   EXP_NEW,
   EXP_CAST,
   EXP_SEQ
} EXP_tag;

union exp {
   EXP_tag tag;
   struct {
      EXP_tag tag;
      int num;
      Exp *e1;
      Exp *e2;
      Type *expt;
   } binexp;
   struct {
      EXP_tag tag;
      int num;
      int val;
      Type *expt;
   } intexp;
   struct {
      EXP_tag tag;
      int len;
      int num;
      char *str;
      Type *expt;
   } strexp;
   struct {
      EXP_tag tag;
      int num;
      float flt;
      Type *expt;
   } fltexp;
   struct {
      EXP_tag tag;
      int num;
      RefVar *r;
   } varexp;
   struct {
      EXP_tag tag;
      Call *c;
      Type *expt;
   } callexp;
   struct {
      EXP_tag tag;
      Exp *e;
      Type *expt;
   } unaryexp;
   struct {
      EXP_tag tag;
      Exp *size;
      Type *expt;
   } newexp;
   struct {
      EXP_tag tag;
      int num;
      Exp *e;
      Type *expt;
   } castexp;
   struct {
      EXP_tag tag;
      Exp *left;
      Exp *right;
   } seqexp;
};

extern Type *typeNode(MT_tag, TYPE_tag);
extern Type *seqtypeNode(MT_tag, Type *);
extern Var *varNode(char *, Type *);
extern Call *callNode(char *, Exp *);
extern RefVar *refvNode(Var *);
extern RefVar *refaNode(Exp *, Exp *);
extern Def *vardefNode(char *, Type *);
extern Def *funcdefNode(Var *, Def *, Stat *);
extern Def *seqdefNode(Def *, Def *);
extern Block *blockNode(Def *, Stat *);
extern Stat *blockstatNode(Def *, Stat *);
extern Stat *ifstatNode(Exp *, Stat *, Stat *);
extern Stat *whilestatNode(Exp *, Stat *);
extern Stat *callstatNode(Call *);
extern Stat *retstatNode(Exp *);
extern Stat *echostatNode(Exp *);
extern Stat *attrstatNode(RefVar *, Exp *);
extern Stat *seqstatNode(Stat *, Stat *);
extern Exp *binexpNode(EXP_tag, Exp *, Exp *);
extern Exp *unaryexpNode(EXP_tag, Exp *);
extern Exp *castexpNode(Exp *, Type *);
extern Exp *newexpNode(Exp *, Type *);
extern Exp *varexpNode(RefVar *);
extern Exp *callexpNode(Call *);
extern Exp *intexpNode(int);
extern Exp *fltexpNode(float);
extern Exp *strexpNode(char *, int);
extern Exp *seqexpNode(Exp *, Exp *);

extern void printTree(Def *);

extern Def *AST_tree;

#endif
