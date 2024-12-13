%locations
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lex.yy.c"

extern int has_error;
extern int last_error_lineno;

void yyerror(char* msg) {
  if(last_error_lineno < yylineno){
    printf("Error type B at Line %d: %s\n", yylineno, msg);
    last_error_lineno = yylineno;
  }
  has_error = 1;
}

%}

%union{
  struct Node* type_node;
}

%token <type_node> SEMI COMMA
%token <type_node> ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR NOT DOT
%token <type_node> LP RP LB RB LC RC
%token <type_node> STRUCT RETURN IF ELSE WHILE TYPE
%token <type_node> INT FLOAT ID

%type <type_node> Program ExtDecList ExtDef ExtDefList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def Dec DecList Exp Args

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGNOP 
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right HIGHER_THAN_MINUS
%right NOT
%left DOT LP RP LB RB

%% 
Program : ExtDefList {Node* c[]={$1}; $$ = createNode(0, 1, "Program", 0, 0, $1->line_no, 1, c); root = $$;}
  | error 
  ;
ExtDefList : ExtDef ExtDefList {Node* c[]={$1, $2}; $$ = createNode(0, 1, "ExtDefList", 0, 0, $1->line_no, 2, c);}
  | {$$ = (Node*)NULL;}
  | error 
  ;
ExtDef : Specifier ExtDecList SEMI {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "ExtDef", 0, 0, $1->line_no, 3, c);}
  | Specifier SEMI {Node* c[]={$1, $2}; $$ = createNode(0, 1, "ExtDef", 0, 0, $1->line_no, 2, c);}
  | Specifier FunDec CompSt {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "ExtDef", 0, 0, $1->line_no, 3, c);}
  | Specifier FunDec SEMI {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "ExtDef", 0, 0, $1->line_no, 3, c);}
  | error SEMI
  ;
ExtDecList : VarDec {Node* c[]={$1}; $$ = createNode(0, 1, "ExtDecList", 0, 0, $1->line_no, 1, c);}
  | VarDec COMMA ExtDecList {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "ExtDecList", 0, 0, $1->line_no, 3, c);}
  | error 
  ;

Specifier : TYPE { Node* c[]={$1}; $$ = createNode(0, 1, "Specifier", 0, 0, $1->line_no, 1, c);}
  | StructSpecifier {Node* c[]={$1}; $$ = createNode(0, 1, "Specifier", 0, 0, $1->line_no, 1, c);}
  | error 
  ;
StructSpecifier : STRUCT OptTag LC DefList RC {Node* c[]={$1, $2, $3, $4, $5}; $$ = createNode(0, 1, "StructSpecifier", 0, 0, $1->line_no, 5, c);}
  | STRUCT Tag {Node* c[]={$1, $2}; $$ = createNode(0, 1, "StructSpecifier", 0, 0, $1->line_no, 2, c);}
  | error RC
  | error SEMI
  ;
OptTag : ID {Node* c[]={$1}; $$ = createNode(0, 1, "OptTag", 0, 0, $1->line_no, 1, c);}
  | {$$ = (Node*)NULL;}
  | error 
  ;
Tag : ID {Node* c[]={$1}; $$ = createNode(0, 1, "Tag", 0, 0, $1->line_no, 1, c);}
  | error 
  ;

VarDec : ID {Node* c[]={$1}; $$ = createNode(0, 1, "VarDec", 0, 0, $1->line_no, 1, c);}
  | VarDec LB INT RB {Node* c[]={$1, $2, $3, $4}; $$ = createNode(0, 1, "VarDec", 0, 0, $1->line_no, 4, c);}
  | error RB
  ;
FunDec : ID LP VarList RP {Node* c[]={$1, $2, $3, $4}; $$ = createNode(0, 1, "FunDec", 0, 0, $1->line_no, 4, c);}
  | ID LP RP {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "FunDec", 0, 0, $1->line_no, 3, c);}
  | error RP
  ;
VarList : ParamDec COMMA VarList {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "VarList", 0, 0, $1->line_no, 3, c);}
  | ParamDec {Node* c[]={$1}; $$ = createNode(0, 1, "VarList", 0, 0, $1->line_no, 1, c);}
  | error 
  ;
ParamDec : Specifier VarDec {Node* c[]={$1, $2}; $$ = createNode(0, 1, "ParamDec", 0, 0, $1->line_no, 2, c);}
  | error 
  ;

CompSt : LC DefList StmtList RC {Node* c[]={$1, $2, $3, $4}; $$ = createNode(0, 1, "CompSt", 0, 0, $1->line_no, 4, c);}
  | error RC
  ;
StmtList : Stmt StmtList {Node* c[]={$1, $2}; $$ = createNode(0, 1, "StmtList", 0, 0, $1->line_no, 2, c);}
  | {$$ = (Node*)NULL;}
  | error 
  ;
Stmt : Exp SEMI {Node* c[]={$1, $2}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 2, c);}
  | CompSt {Node* c[]={$1}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 1, c);}
  | RETURN Exp SEMI {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 3, c);}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {Node* c[]={$1, $2, $3, $4, $5}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 5, c);}
  | IF LP Exp RP Stmt ELSE Stmt {Node* c[]={$1, $2, $3, $4, $5, $6, $7}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 7, c);}
  | WHILE LP Exp RP Stmt {Node* c[]={$1, $2, $3, $4, $5}; $$ = createNode(0, 1, "Stmt", 0, 0, $1->line_no, 5, c);}
  | WHILE LP Exp error Stmt
  | error SEMI
  | error RP
  ;

DefList : Def DefList {Node* c[]={$1, $2}; $$ = createNode(0, 1, "DefList", 0, 0, $1->line_no, 2, c);}
  | {$$ = (Node*)NULL;}
  | error 
  ;
Def : Specifier DecList SEMI {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Def", 0, 0, $1->line_no, 3, c);}
  | error SEMI
  ;
DecList : Dec {Node* c[]={$1}; $$ = createNode(0, 1, "DecList", 0, 0, $1->line_no, 1, c);}
  | Dec COMMA DecList {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "DecList", 0, 0, $1->line_no, 3, c);}
  | error 
  ;
Dec : VarDec {Node* c[]={$1}; $$ = createNode(0, 1, "Dec", 0, 0, $1->line_no, 1, c);}
  | VarDec ASSIGNOP Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Dec", 0, 0, $1->line_no, 3, c);}
  | error 
  ;

Exp : Exp ASSIGNOP Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp AND Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp OR Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp RELOP Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp PLUS Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp MINUS Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp STAR Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp DIV Exp {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | LP Exp RP {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | MINUS Exp %prec HIGHER_THAN_MINUS {Node* c[]={$1, $2}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 2, c);}
  | NOT Exp {Node* c[]={$1, $2}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 2, c);}
  | ID LP Args RP {Node* c[]={$1, $2, $3, $4}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 4, c);}
  | ID LP RP {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c);}
  | Exp LB Exp RB {Node* c[]={$1, $2, $3, $4}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 4, c); $$->can_left = 1;}
  | Exp DOT ID {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 3, c); $$->can_left = 1;}
  | ID {Node* c[]={$1}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 1, c); $$->can_left = 1;}
  | INT {Node* c[]={$1}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 1, c);}
  | FLOAT {Node* c[]={$1}; $$ = createNode(0, 1, "Exp", 0, 0, $1->line_no, 1, c);}
  | error
  ;
Args : Exp COMMA Args {Node* c[]={$1, $2, $3}; $$ = createNode(0, 1, "Args", 0, 0, $1->line_no, 3, c);}
  | Exp {Node* c[]={$1}; $$ = createNode(0, 1, "Args", 0, 0, $1->line_no, 1, c);}
  | error 
  ;
%%