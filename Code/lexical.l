%option yylineno
%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "syntax.tab.h"

int has_error = 0;
int last_error_lineno = 0;

%}
digit [0-9]
letter_ [_a-zA-Z]
delim [ \t\r\n]
%% 
"//".* { }
"/*" { int c1 = 0, c2 = input ();
        while(1){
          if(c2 == EOF || c2 == 0){
              if(last_error_lineno < yylineno){
                printf("Error type B at Line %d: \n", yylineno);
                last_error_lineno = yylineno;
              }
              has_error = 1;  
              break;
          }
          if(c1 == '*' && c2 == '/')
            break;
          c1 = c2;
          c2 = input();
        }
      }
{delim}+ { }
";" { yylval.type_node = createNode(1, 1, "SEMI", 0, 0, yylineno, 0, NULL); return SEMI; }
"," { yylval.type_node = createNode(1, 1, "COMMA", 0, 0, yylineno, 0, NULL); return COMMA; }
"=" { yylval.type_node = createNode(1, 1, "ASSIGNOP", 0, 0, yylineno, 0, NULL); return ASSIGNOP; }
">"|"<"|">="|"<="|"=="|"!=" { char str[40] = "RELOP: "; yylval.type_node = createNode(1, 1, strcat(str, yytext), 0, 0, yylineno, 0, NULL); return RELOP; }
"+" { yylval.type_node = createNode(1, 1, "PLUS", 0, 0, yylineno, 0, NULL); return PLUS; }
"-" { yylval.type_node = createNode(1, 1, "MINUS", 0, 0, yylineno, 0, NULL); return MINUS; }
"*" { yylval.type_node = createNode(1, 1, "STAR", 0, 0, yylineno, 0, NULL); return STAR; }
"/" { yylval.type_node = createNode(1, 1, "DIV", 0, 0, yylineno, 0, NULL); return DIV; }
"&&" { yylval.type_node = createNode(1, 1, "AND", 0, 0, yylineno, 0, NULL); return AND; }
"||" { yylval.type_node = createNode(1, 1, "OR", 0, 0, yylineno, 0, NULL); return OR; }
"." { yylval.type_node = createNode(1, 1, "DOT", 0, 0, yylineno, 0, NULL); return DOT; }
"!" { yylval.type_node = createNode(1, 1, "NOT", 0, 0, yylineno, 0, NULL); return NOT; }
"(" { yylval.type_node = createNode(1, 1, "LP", 0, 0, yylineno, 0, NULL); return LP; }
")" { yylval.type_node = createNode(1, 1, "RP", 0, 0, yylineno, 0, NULL); return RP; }
"[" { yylval.type_node = createNode(1, 1, "LB", 0, 0, yylineno, 0, NULL); return LB; }
"]" { yylval.type_node = createNode(1, 1, "RB", 0, 0, yylineno, 0, NULL); return RB; }
"{" { yylval.type_node = createNode(1, 1, "LC", 0, 0, yylineno, 0, NULL); return LC; }
"}" { yylval.type_node = createNode(1, 1, "RC", 0, 0, yylineno, 0, NULL); return RC; }
struct { yylval.type_node = createNode(1, 1, "STRUCT", 0, 0, yylineno, 0, NULL); return STRUCT; }
return { yylval.type_node = createNode(1, 1, "RETURN", 0, 0, yylineno, 0, NULL); return RETURN; }
if { yylval.type_node = createNode(1, 1, "IF", 0, 0, yylineno, 0, NULL); return IF; }
else { yylval.type_node = createNode(1, 1, "ELSE", 0, 0, yylineno, 0, NULL); return ELSE; }
while { yylval.type_node = createNode(1, 1, "WHILE", 0, 0, yylineno, 0, NULL); return WHILE; }
int|float { char str[40] = "TYPE: "; yylval.type_node = createNode(1, 1, strcat(str, yytext), 0, 0, yylineno, 0, NULL); return TYPE; }
{digit}+ { yylval.type_node = createNode(1, 2, "", atoi(yytext), 0, yylineno, 0, NULL); return INT; }
{digit}+"."{digit}+ { yylval.type_node = createNode(1, 3, "", 0, atof(yytext), yylineno, 0, NULL); return FLOAT; }
{letter_}({letter_}|{digit})* { char str[40] = "ID: "; yylval.type_node = createNode(1, 1, strcat(str, yytext), 0, 0, yylineno, 0, NULL); yylval.type_node->can_left = 1; return ID; }
. {
    if(last_error_lineno < yylineno){
      printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
      last_error_lineno = yylineno;
    }
    has_error = 1;
}
%%