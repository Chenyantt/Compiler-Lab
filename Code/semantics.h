#ifndef __SEMANTICS_H__
#define __SEMANTICS_H__

#include "ast.h"
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Entry_* Entry;
typedef struct Frame_* Frame;

struct Type_
{
  enum { BASIC, ARRAY, STRUCTURE, ERROR } kind;
  union
  {
    int basic;
    struct { Type elem; int size; } array;
    struct{
      char name[40];
      FieldList structure;
    };
  } u;
};

struct FieldList_
{
  char name[40]; 
  Type type; 
  FieldList tail; 
};

struct Entry_
{
    enum {VAR, FUNC} kind;
    char name[40];
    union{
      struct{
        Type type;
        int is_param;
      }u;
      struct{
        Type returntype;
        FieldList paras;
        int line_no;
        int is_impl;
      }v;
    };
    Entry next;
};

Entry symtab_h;

struct Frame_
{
    char name[40];
    Type type;
    Frame next;
};

Frame sttab_h;

Type build_type(int kind, int basic, Type elem, int size, FieldList structure, char name[]);
void insert_entry(int kind, char name[], Type type, FieldList paras);
Entry lookup_symtab(Entry head, char name[]);
Entry lookup_func(Entry head, char name[]);
void insert_frame(char name[], Type type);
Frame lookup_sttab(Frame head, char name[]);
void analyse(Node* root);
void do_extdef(Node* root);
int do_fundec(Type returntype, Node* root);
FieldList do_varlist(Node* root);
FieldList do_paramdec(Node* root, FieldList tail);
void do_extdeclist(Type t, Node* root);
Type do_specifier(Node* root);
Type do_struct_specifier(Node* root);
FieldList do_deflist(Node* root, FieldList tail, int in_struct);
FieldList do_def(Node* root, FieldList tail, int in_struct);
FieldList do_declist(Type t, Node* root, FieldList tail, int in_struct);
FieldList do_dec(Type t, Node* root, FieldList tail, int in_struct);
FieldList do_vardec(Node* root, FieldList tail1, Type tail2, int in_struct);
void do_compst(Node* root, Type returntype);
void do_stmtlist(Node* root, Type returntype);
void do_stmt(Node* root, Type returntype);
int is_same_type(Type a, Type b);
int is_same_paras(FieldList f, Node* root);
Type find_field(FieldList f, char name[]);
Type do_expr(Node* root);

int is_same_field(FieldList a, FieldList b);
void do_funcla(Type returntype, Node* root);
FieldList do_varlistcla(Node* root);
FieldList do_paramcla(Node* root, FieldList tail);
FieldList do_varcla(Node* root, FieldList tail1, Type tail2, int in_struct);
void check(Entry head);
void semantics();

#endif