%token TK_INT TK_CHAR TK_FLOAT TK_VOID
%token TK_IF TK_ELSE TK_WHILE TK_RETURN TK_NEW TK_AS
%token TK_ID TK_STRING TK_LITERAL TK_SFLOAT TK_SINT
%token TK_AND TK_OR TK_GEQUALS TK_LEQUALS TK_EQUALS TK_NEQUALS

%define parse.trace true
%error-verbose

%union SValue {
   float fnum;
   int inum;
   char *str;
}

%{
#include <stdio.h>
#include "lex.yy.h"

void yyerror(const char *);
%}

%%

programa : function-list
         | def-vars function-list
         ;

function-list : %empty
              | def-function def-vars function-list
              | def-function function-list
              ;

def-var : id-list ':' type ';'
        ;

def-vars : def-var
         | def-vars def-var
         ;

var : TK_ID
    ;

id-list : var
        | var ',' id-list
        ;

primary-type : TK_INT
             | TK_CHAR
             | TK_FLOAT
             | TK_VOID
             ;

type : primary-type
     | type '[' ']'
     ;

def-function : var '(' parameters ')' ':' type statement
             ;

parameters : %empty
           | parameter ',' parameters
           | parameter
           ;

parameter : var ':' type
          ;

statement : '{' '}'
          | closed-statement
          ;

closed-statement : '{' def-vars closedcmd-list '}'
                 | '{' closedcmd-list '}'
                 | '{' def-vars '}'
                 ;

closedcmd-list : closed-command
               | closed-command closedcmd-list
               ;

closed-command: TK_IF '(' exp ')' statement
              | TK_IF '(' exp ')' statement TK_ELSE statement
              | TK_WHILE '(' exp ')' statement
              | exp ';'
              | TK_RETURN ';'
              | TK_RETURN exp ';'
              | closed-statement
              ;

exp : infix-exp
    | prefix-exp '=' subadd-exp
    | '@' infix-exp
    ;

infix-exp : infix-exp TK_OR equal-exp
          | infix-exp TK_AND equal-exp
          | equal-exp
          ;

equal-exp : equal-exp TK_EQUALS relation-exp
          | equal-exp TK_NEQUALS relation-exp
          | relation-exp
          ;

relation-exp : relation-exp '<' subadd-exp
             | relation-exp '>' subadd-exp
             | relation-exp TK_GEQUALS subadd-exp
             | relation-exp TK_LEQUALS subadd-exp
             | subadd-exp
             ;

subadd-exp : subadd-exp '+' multdiv-exp
           | subadd-exp '-' multdiv-exp
           | multdiv-exp
           ;

multdiv-exp : multdiv-exp '*' prefix-exp
            | multdiv-exp '/' prefix-exp
            | prefix-exp
            ;

prefix-exp : '-' posfix-exp
           | '!' posfix-exp
           | posfix-exp
           ;

posfix-exp : primary
           | func-call
           | posfix-exp '[' subadd-exp ']'
           | posfix-exp  TK_AS primary-type
           | TK_NEW type '[' subadd-exp ']'
           ;

func-call : var '(' list-exp ')'
          | var '(' ')'
          ;

list-exp  : infix-exp
          | infix-exp ',' list-exp
          ;

number : TK_SINT
       | TK_SFLOAT
       ;

word : TK_LITERAL
     | TK_STRING
     ;

primary : word
        | number
        | var
        | '(' exp ')'
        ;
%%

void yyerror(const char *s)
{
   fprintf(stderr, "%s\n", s);
}
