#ifndef __IR_H__
#define __IR_H__

#include <stdio.h>
#include "ast.h"

typedef struct Arglist_* Arglist;
struct Arglist_
{
  char* name; 
  Arglist next; 
};

int trans_ir(FILE* out);
void trans_dec(Node* root);
void trans_declist(Node* root);
void trans_def(Node* root);
void trans_deflist(Node* root);
void trans_stmtlist(Node* root);
void trans_compst(Node* root);
void trans_extdef(Node* root);
void translate(Node* root);
void trans_cond(Node* root, char* lt, char* lf);
void trans_stmt(Node* root);
void trans_expr(Node* root, char* place);
void assign_array(char* a, char* b, int sz);
Arglist trans_args(Node* root, Arglist head);

#endif