#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

Node* createNode(int is_token, int type, const char* str, int ino, float fno, int line_no, int child_no, Node* c[]){
  Node* node = (Node*)malloc(sizeof(Node));
  node->is_token = is_token;
  node->type = type;
  node->line_no = line_no;
  if(type == 1) strcpy(node->str, str);
  else if(type==2) node->ino = ino;
  else node->fno = fno;
  node->child_no = child_no;
  if(c != NULL) memcpy(node->childs, c, sizeof(Node*) * child_no);
  return node;
}

void printTree(Node* root, int depth){
  for(int i=0;i<depth;++i) printf("  ");
  if(root->type == 1) printf("%s", root->str);
  else if(root->type==2) printf("INT: %d", root->ino);
  else printf("FLOAT: %f", root->fno);
  if(!root->is_token) printf(" (%d)",root->line_no);
  puts("");
  for(int i = 0; i < root->child_no; ++i)
    if(root->childs[i] != NULL) printTree(root->childs[i], depth + 1);
}