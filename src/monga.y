%token TK_INT TK_CHAR TK_FLOAT TK_VOID
%token TK_IF TK_ELSE TK_WHILE TK_RETURN TK_NEW TK_AS
%token TK_AND TK_OR TK_GEQUALS TK_LEQUALS TK_EQUALS TK_NEQUALS
%token <wrap> TK_ID
%token <wrap> TK_STRING
%token <inum> TK_VINT
%token <fnum> TK_VFLOAT

%define parse.trace true
%error-verbose

%code requires {
#include "tree.h"

struct wrap {
   char *str;
   int len;
};

}

%{
#include <stdio.h>
#include "lex.yy.h"

void yyerror(const char *);

%}

%union {
   Type *utype;
   Exp *uexp;
   Stat *ustat;
   RefVar *urefvar;
   Def *udef;
   Call *ucall;
   int inum;
   struct wrap wrap;
   float fnum;
}

%type <ucall> func-call
%type <utype> primary-type type
%type <urefvar> var
%type <udef> function-list def-function def-var def-vars
             parameter parameters parameters1
%type <ustat> statement commandlst command
%type <uexp> exp or-exp and-exp relation-exp equal-exp subadd-exp multdiv-exp
             base-exp prefix-exp posfix-exp primary number word list-exp list1-exp
             new-exp

%%

programa : def-vars function-list { AST_tree = seqdefNode($1, $2); }
         ;

function-list : %empty                          { $$ = NULL; }
              | def-function function-list      { $$ = seqdefNode($1, $2); }
              ;

def-var : TK_ID ':' type ';'    { $$ = vardefNode($1.str, $3); }
        ;

def-vars : %empty               { $$ = NULL; }
         | def-vars def-var     { $$ = seqdefNode($1, $2); }
         ;

primary-type : TK_INT           { $$ = typeNode(ATOMIC, INT); }
             | TK_CHAR          { $$ = typeNode(ATOMIC, CHAR); }
             | TK_FLOAT         { $$ = typeNode(ATOMIC, FLOAT); }
             | TK_VOID          { $$ = typeNode(ATOMIC, VOID); }
             ;

type : primary-type     { $$ = $1; }
     | type '[' ']'     { $$ = seqtypeNode(SEQ, $1); }
     ;

def-function : TK_ID '(' parameters ')' ':' type statement { $$ = funcdefNode(varNode($1.str, $6), $3, $7); }
             ;

parameters : %empty                     { $$ = NULL; }
           | parameter parameters1      { $$ = seqdefNode($1, $2); }
           ;

parameters1 : %empty                    { $$ = NULL; }
            | ',' parameter parameters1 { $$ = seqdefNode($2, $3); }

parameter : TK_ID ':' type { $$ = vardefNode($1.str, $3); }
          ;

statement : '{' def-vars commandlst '}' { $$ = blockstatNode($2, $3); }
          ;

commandlst : %empty     { $$ = NULL; }
           | command commandlst { $$ = seqstatNode($1, $2); }
           ;

var : TK_ID                             { $$ = refvNode(varNode($1.str, NULL)); }
    | base-exp '[' subadd-exp ']'       { $$ = refaNode($1, $3); }
    ;

command: TK_IF '(' exp ')' statement      { $$ = ifstatNode($3, $5, NULL); }
              | TK_IF '(' exp ')' statement TK_ELSE statement   { $$ = ifstatNode($3, $5, $7); }
              | TK_WHILE '(' exp ')' statement  { $$ = whilestatNode($3, $5); }
              | func-call ';'           { $$ = callstatNode($1); }
              | TK_RETURN ';'           { $$ = retstatNode(NULL); }
              | TK_RETURN or-exp ';'    { $$ = retstatNode($2); }
              | '@' exp ';'             { $$ = echostatNode($2); }
              | var '=' exp ';'         { $$ = attrstatNode($1, $3); }
              | statement               { $$ = $1; }
              ;

exp : or-exp    { $$ = $1; }
    ;

or-exp : or-exp TK_OR and-exp   { $$ = binexpNode(EXP_OR, $1, $3); }
       | and-exp                { $$ = $1; }
       ;

and-exp : and-exp TK_AND equal-exp      { $$ = binexpNode(EXP_AND, $1, $3); }
        | equal-exp                     { $$ = $1; }
        ;

equal-exp : relation-exp TK_EQUALS relation-exp         { $$ = binexpNode(EXP_EQ, $1, $3); }
          | relation-exp TK_NEQUALS relation-exp        { $$ = binexpNode(EXP_NEQ, $1, $3); }
          | relation-exp                                { $$ = $1; }
          ;

relation-exp : subadd-exp '<' subadd-exp                { $$ = binexpNode(EXP_LESS, $1, $3); }
             | subadd-exp '>' subadd-exp                { $$ = binexpNode(EXP_GRE, $1, $3); }
             | subadd-exp TK_GEQUALS subadd-exp         { $$ = binexpNode(EXP_GEQ, $1, $3); }
             | subadd-exp TK_LEQUALS subadd-exp         { $$ = binexpNode(EXP_LEQ, $1, $3); }
             | subadd-exp                               { $$ = $1; }
             ;

subadd-exp : subadd-exp '+' multdiv-exp                 { $$ = binexpNode(EXP_ADD, $1, $3); }
           | subadd-exp '-' multdiv-exp                 { $$ = binexpNode(EXP_SUB, $1, $3); }
           | multdiv-exp                                { $$ = $1; }
           ;

multdiv-exp : multdiv-exp '*' prefix-exp                { $$ = binexpNode(EXP_MUL, $1, $3); }
            | multdiv-exp '/' prefix-exp                { $$ = binexpNode(EXP_DIV, $1, $3); }
            | prefix-exp                                { $$ = $1; }
            ;

prefix-exp : '-' prefix-exp     { $$ = unaryexpNode(EXP_UNARYMINUS, $2); }
           | '!' prefix-exp     { $$ = unaryexpNode(EXP_UNARYNOT, $2); }
           | posfix-exp         { $$ = $1; }
           ;

posfix-exp : base-exp                         { $$ = $1; }
           | base-exp  TK_AS type             { $$ = castexpNode($1, $3); }
           | new-exp                          { $$ = $1; }
           ;

new-exp : TK_NEW type '[' subadd-exp ']'          { $$ = newexpNode($4, $2); }
        ;

base-exp : primary      { $$ = $1; }
         | func-call    { $$ = callexpNode($1); }
         ;

func-call : TK_ID '(' list-exp ')' { $$ = callNode($1.str, $3); }
          ;

list-exp  : %empty              { $$ = NULL; }
          | or-exp list1-exp    { $$ = seqexpNode($1, $2); }
          ;

list1-exp : %empty                      { $$ = NULL; }
          | ',' or-exp list1-exp        { $$ = seqexpNode($2, $3); }
          ;

number : TK_VINT        { $$ = intexpNode($1); }
       | TK_VFLOAT      { $$ = fltexpNode($1); }
       ;

word : TK_STRING { $$ = strexpNode($1.str, $1.len); }
     ;

primary : word          { $$ = $1; }
        | number        { $$ = $1; }
        | var           { $$ = varexpNode($1); }
        | '(' exp ')'   { $$ = $2; }
        ;
%%

void yyerror(const char *s)
{
   fprintf(stderr, "%s\n", s);
}
