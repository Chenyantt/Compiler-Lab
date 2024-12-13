#ifndef __AST_H__
#define __AST_H__

typedef struct Node{
  int is_token;
  int type;
  union{
    char str[40];
    int ino;
    float fno;
  };
  int line_no;
  int child_no;
  struct Node* childs[7];
  int can_left;
}Node;

Node* createNode(int is_token, int type, const char* str, int ino, float fno, int line_no, int child_no, Node* c[]);

void printTree(Node* root, int depth);

Node* root;

#endif