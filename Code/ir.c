#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "semantics.h"

FILE *f;

int tag = 0;

int tempnum = 0, labelnum = 0;

char * new_temp(){
  char* str = malloc(40);
  sprintf(str, "t%d", ++tempnum);
  return str;
}

char * new_label(){
  char* str = malloc(40);
  sprintf(str, "label%d", ++labelnum);
  return str;
}

int get_size(Type t){
  int res = 1;
  while(t->kind != BASIC){
    res *= t->u.array.size;
    t = t->u.array.elem;
  }
  return res;
}

int trans_array(Node* root, char* place){
  Node* tmp = root;
  Arglist head = NULL;
  while(tmp->child_no > 1){
    Arglist n = malloc(sizeof(struct Arglist_));
    n->name = new_temp();
    trans_expr(tmp->childs[2], n->name);
    n->next = head;
    head = n;
    tmp = tmp->childs[0];
  }
  tmp = tmp->childs[0];
  Entry e = lookup_symtab(symtab_h, tmp->str + 4);
  Type t = e->u.type;
  char* sum = new_temp();
  fprintf(f, "%s := #0\n", sum);
  while(head){
    fprintf(f, "%s := %s * #%d\n", sum, sum, t->u.array.size);
    fprintf(f, "%s := %s + %s\n", sum, sum, head->name);
    t = t->u.array.elem;
    head = head->next;
  }
  if(t->kind != BASIC)
     fprintf(f, "%s := %s * #%d\n", sum, sum, get_size(t));
  char* off = new_temp();
  fprintf(f, "%s := %s * #4\n", off, sum);
  if(e->u.is_param){
    fprintf(f, "%s := %s + %s\n", place, tmp->str + 4, off);
  }else{
    fprintf(f, "%s := &%s + %s\n", place, tmp->str + 4, off);
  }
  if(t->kind != BASIC) return get_size(t);
  return 0;
}

Arglist trans_args(Node* root, Arglist head){
  char* t1 = new_temp();
  trans_expr(root->childs[0], t1);
  Arglist n = malloc(sizeof(struct Arglist_));
  n->name = t1,n->next = head;
  if(root->child_no == 1) return n;
  else return trans_args(root->childs[2], n);
}

void assign_array(char* a, char* b, int sz){
  char* t1 = new_temp(), *t2 = new_temp();
  fprintf(f, "%s := %s\n", t1, a);
  fprintf(f, "%s := %s\n", t2, b);
  for(int i=0;i<sz;++i){
    fprintf(f, "*%s := *%s\n", t1, t2);
    fprintf(f, "%s := %s + #4\n", t1, t1);
    fprintf(f, "%s := %s + #4\n", t2, t2);
  }
}

void trans_expr(Node* root, char* place){
  if(root->child_no == 1){
    if(root->childs[0]->type == 1){
      if(place){
        Entry e = lookup_symtab(symtab_h, root->childs[0]->str + 4);
        if(e->u.type->kind == BASIC || e->u.is_param)
          fprintf(f, "%s := %s\n", place, root->childs[0]->str + 4);
        else
          fprintf(f, "%s := &%s\n", place, root->childs[0]->str + 4);
      }
    }else{
      if(place) fprintf(f, "%s := #%d\n", place, root->childs[0]->ino);
    }
  }
  else if(strcmp(root->childs[1]->str, "ASSIGNOP") == 0){
      char* str;
      char tmp[40];
      int is_arr = 0;
      if(root->childs[0]->child_no == 1){
        str = root->childs[0]->childs[0]->str + 4;
        Entry e = lookup_symtab(symtab_h, str);
        if(e->u.type->kind != BASIC){
          is_arr = get_size(e->u.type);
          if(!e->u.is_param){
            tmp[0]= '&';
            strcpy(tmp + 1, str);
            str = tmp;
          }
        }
      } 
      else{
        str = new_temp();
        is_arr = trans_array(root->childs[0], str);
      } 
      char* t1 = new_temp();
      trans_expr(root->childs[2], t1);
      if(is_arr){
        assign_array(str, t1, is_arr);
      }else if(root->childs[0]->child_no != 1){
        fprintf(f, "*%s := %s\n",str, t1);
      }else{
        fprintf(f, "%s := %s\n",str, t1);
      }
      if(place) fprintf(f, "%s := %s\n",place, t1);
  }
  else if(strcmp(root->childs[0]->str, "NOT") == 0 || strcmp(root->childs[1]->str, "AND") == 0 || 
    strcmp(root->childs[1]->str, "OR") == 0 || root->childs[1]->str[0] == 'R'){
      char* l1 = new_label(), *l2 = new_label();
      if(place) fprintf(f, "%s := #0\n", place);
      trans_cond(root, l1, l2);
      fprintf(f, "LABEL %s :\n", l1);
      if(place) fprintf(f, "%s := #1\n", place);
      fprintf(f, "LABEL %s :\n", l2);
  }else if(root->child_no == 2){
    char* t1 = new_temp();
    trans_expr(root->childs[1], t1);
    if(place) fprintf(f, "%s := #0 - %s\n",place, t1);
  }else if(root->child_no == 3 && strcmp(root->childs[2]->str, "Exp") == 0){
    char* t1 = new_temp(), *t2 = new_temp();
    trans_expr(root->childs[0], t1), trans_expr(root->childs[2], t2);
    if(!place) return;
    if(root->childs[1]->str[0]== 'P') fprintf(f, "%s := %s + %s\n", place, t1, t2);
    else if(root->childs[1]->str[0]== 'M') fprintf(f, "%s := %s - %s\n", place, t1, t2);
    else if(root->childs[1]->str[0]== 'S') fprintf(f, "%s := %s * %s\n", place, t1, t2);
    else fprintf(f, "%s := %s / %s\n", place, t1, t2);
  }else if(strcmp(root->childs[1]->str, "LB") == 0){
    char* str = new_temp(); 
    int is_arr = trans_array(root, str);
    if(place){
      if(is_arr)
        fprintf(f, "%s := %s\n", place, str);
      else
        fprintf(f, "%s := *%s\n", place, str);
    }
  }else if(strcmp(root->childs[0]->str, "LP") == 0){
    trans_expr(root->childs[1], place);
  }else if(root->child_no == 3){
    if(strcmp(root->childs[0]->str + 4, "read") == 0){
      fprintf(f, "READ %s\n", place);
    }else{
      if(place) fprintf(f, "%s := CALL %s\n", place, root->childs[0]->str + 4);
    }
  }else{
     Arglist head = trans_args(root->childs[2], NULL);
     if(strcmp(root->childs[0]->str + 4, "write") == 0){
      fprintf(f, "WRITE %s\n", head->name);
      fprintf(f, "%s := #0\n",place);
     }else{
      while(head){
        fprintf(f, "ARG %s\n", head->name);
        head = head->next;
      }
      fprintf(f, "%s := CALL %s\n",place, root->childs[0]->str + 4);
     }
  }
}

void trans_stmt(Node* root){
  if(root->child_no == 1){
    trans_compst(root->childs[0]);
  }else if(root->child_no == 2){
    trans_expr(root->childs[0], NULL);
  }else if(root->child_no == 3){
    char* t1 = new_temp();
    trans_expr(root->childs[1], t1);
    fprintf(f, "RETURN %s\n", t1);
  }else if(root->child_no == 7){
    char* l1 = new_label(), *l2 =new_label(), *l3 = new_label();
    trans_cond(root->childs[2], l1, l2);
    fprintf(f, "LABEL %s :\n", l1);
    trans_stmt(root->childs[4]);
    fprintf(f, "GOTO %s\nLABEL %s :\n", l3, l2);
    trans_stmt(root->childs[6]);
    fprintf(f, "LABEL %s :\n", l3);
  }else if(strcmp(root->childs[0]->str, "IF") == 0){
    char* l1 = new_label(), *l2 =new_label();
    trans_cond(root->childs[2], l1, l2);
    fprintf(f, "LABEL %s :\n", l1);
    trans_stmt(root->childs[4]);
    fprintf(f, "LABEL %s :\n", l2);
  }else{
    char* l1 = new_label(), *l2 = new_label(), *l3 = new_label();
    fprintf(f, "LABEL %s :\n", l1);
    trans_cond(root->childs[2], l2, l3);
    fprintf(f, "LABEL %s :\n", l2);
    trans_stmt(root->childs[4]);
    fprintf(f, "GOTO %s\nLABEL %s :\n", l1, l3);
  }
}

void trans_cond(Node* root, char* lt, char* lf){
  if(root->child_no >= 2){
    if(strcmp(root->childs[1]->str, "AND") == 0 ){
      char* l1 = new_label();
      trans_cond(root->childs[0], l1, lf);
      fprintf(f, "LABEL %s :\n", l1);
      trans_cond(root->childs[2], lt, lf);
      return;
    }else if(strcmp(root->childs[1]->str, "OR") == 0){
      char* l1 = new_label();
      trans_cond(root->childs[0], lt, l1);
      fprintf(f, "LABEL %s :\n", l1);
      trans_cond(root->childs[2], lt, lf);
      return;
    }else if(strcmp(root->childs[0]->str, "NOT") == 0){
      trans_cond(root->childs[1], lf, lt);
      return;
    }else if(strncmp(root->childs[1]->str, "RELOP", 5) == 0){
      char* t1 = new_temp(), *t2 =new_temp();
      trans_expr(root->childs[0], t1), trans_expr(root->childs[2], t2);
      fprintf(f, "IF %s %s %s GOTO %s\nGOTO %s\n", t1, root->childs[1]->str + 7, t2, lt, lf);
      return;
    }
  }
  char* t1 = new_temp();
  trans_expr(root, t1);
  fprintf(f, "IF %s != #0 GOTO %s\nGOTO %s\n", t1, lt, lf);
}

void translate(Node* root){
  if(root->type == 1){
    if(strcmp(root->str,"ExtDef") == 0){
        trans_extdef(root);
        return;
    }
  }
  for(int i = 0; i < root->child_no; ++i)
    if(root->childs[i] != NULL) 
        translate(root->childs[i]);
}

void trans_extdef(Node* root){
  if(tag || strcmp(root->childs[1]->str, "FunDec") != 0){
    tag = 1;
    return;
  }
    fprintf(f, "FUNCTION %s :\n", root->childs[1]->childs[0]->str + 4);
    Entry e = lookup_func(symtab_h, root->childs[1]->childs[0]->str + 4);
    FieldList paras = e->v.paras;
    while(paras){
      fprintf(f, "PARAM %s\n", paras->name);
      paras = paras->tail;
    }
    trans_compst(root->childs[2]);
}

void trans_compst(Node* root){
  if(root->childs[1] != NULL)
      trans_deflist(root->childs[1]);
  if(root->childs[2] != NULL)
      trans_stmtlist(root->childs[2]);
}

void trans_stmtlist(Node* root){
  trans_stmt(root->childs[0]);
  if(root->childs[1] ) trans_stmtlist(root->childs[1]);
}

void trans_deflist(Node* root){
  trans_def(root->childs[0]);
  if(root->childs[1] != NULL)
      trans_deflist(root->childs[1]);
}

void trans_def(Node* root){
  trans_declist(root->childs[1]);
}

void trans_declist(Node* root){
  trans_dec(root->childs[0]);
  if(root->child_no > 1) trans_declist(root->childs[2]);
}

void trans_dec(Node* root){
  Node* tmp = root->childs[0];
  while(tmp->child_no > 1) tmp = tmp->childs[0];
  tmp = tmp->childs[0];
  Entry e  = lookup_symtab(symtab_h, tmp->str + 4);
  if(e->u.type->kind != BASIC){
    int sz = get_size(e->u.type);
    fprintf(f, "DEC %s %d\n", tmp->str + 4, 4 * sz);
     if(root->child_no > 1){
      char* t1 = new_temp(), *t2 = new_temp();
      trans_expr(root->childs[2], t1);
      fprintf(f, "%s := &%s", t2, tmp->str + 4);
      assign_array(t2, t1, sz);
    }
  }else{
    if(root->child_no > 1){
      char* t1 = new_temp();
      trans_expr(root->childs[2], t1);
      fprintf(f, "%s := %s\n",tmp->str + 4, t1);
    }
  }
}

int trans_ir(FILE* out){
    f = out;
    translate(root);
    return tag;
}