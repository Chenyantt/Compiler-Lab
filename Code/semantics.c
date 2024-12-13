#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "ast.h"

char random_name[40] = "00niming";
int is_error[1000];

Type build_type(int kind, int basic, Type elem, int size, FieldList structure, char name[]){
    Type t=(Type)malloc(sizeof(struct Type_));
    memset(t, 0, sizeof(struct Type_));
    t->kind = kind;
    if(kind == 0){
        t->u.basic = basic;
    }else if(kind == 1){
        t->u.array.elem = elem;
        t->u.array.size = size;
    }else if(kind == 2){
        t->u.structure = structure;
        strcpy(t->u.name, name);
    }
    return t;
}

void insert_entry(int kind, char name[], Type type, FieldList paras){
    Entry e = (Entry)malloc(sizeof(struct Entry_));
    memset(e, 0, sizeof(struct Entry_));
    e->kind = kind;
    strcpy(e->name, name);
    if(kind == VAR){
        e->u.type = type;
    }else{
        e->v.returntype = type;
        e->v.paras = paras;   
    }
    e->next = symtab_h;
    symtab_h = e;
}

Entry lookup_symtab(Entry head, char name[]){
    if(head == NULL) return NULL;
    else if(head->kind == VAR && strcmp(head->name, name) == 0) return head;
    else return lookup_symtab(head->next, name);
}

Entry lookup_func(Entry head, char name[]){
    if(head == NULL) return NULL;
    else if(head->kind == FUNC && strcmp(head->name, name) == 0) return head;
    else return lookup_func(head->next, name);
}

void insert_frame(char name[], Type type){
    Frame e = (Frame)malloc(sizeof(struct Frame_));
    strcpy(e->name, name);
    e->type = type;
    e->next = sttab_h;
    sttab_h = e;
}

Frame lookup_sttab(Frame head, char name[]){
    if(head == NULL) return NULL;
    else if(strcmp(head->name, name) == 0) return head;
    else return lookup_sttab(head->next, name);
}

void analyse(Node* root){
  if(root->type == 1){
    if(strcmp(root->str,"ExtDef") == 0){
        do_extdef(root);
        return;
    }
  }
  for(int i = 0; i < root->child_no; ++i)
    if(root->childs[i] != NULL) 
        analyse(root->childs[i]);
}

void do_extdef(Node* root){
    Type t = do_specifier(root->childs[0]);
    if(root->child_no > 2){
        if(strcmp(root->childs[2]->str, "SEMI") == 0){
            if(strcmp(root->childs[1]->str, "FunDec") == 0){
                do_funcla(t, root->childs[1]);
            }
            else do_extdeclist(t, root->childs[1]);
        }else{
            if(do_fundec(t, root->childs[1]))
                do_compst(root->childs[2], t);
        }
    }
}

int is_same_field(FieldList a, FieldList b){
    if(a==NULL && b==NULL) return 1;
    if(a==NULL || b==NULL || !is_same_type(a->type,b->type)) return 0;
    return is_same_field(a->tail,b->tail);
}

void do_funcla(Type returntype, Node* root){
    FieldList f = NULL;
    if(root->child_no >3){
        f = do_varlistcla(root->childs[2]);
    } 
    Entry e = lookup_func(symtab_h, root->childs[0]->str + 4);
    if(e == NULL){
        insert_entry(FUNC, root->childs[0]->str + 4, returntype, f);
        symtab_h->v.line_no = root->line_no;
        symtab_h->v.is_impl = 0;
    }
    else if(!is_same_field(e->v.paras, f) || !is_same_type(e->v.returntype, returntype)){
        if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 19 at Line %d: \n", root->line_no);  
    }   
}

FieldList do_varlistcla(Node* root){
    FieldList f = NULL;
    if(root->child_no > 1){
        f = do_varlistcla(root->childs[2]);
    }
    return do_paramcla(root->childs[0], f);
}

FieldList do_paramcla(Node* root, FieldList tail){
    Type t =do_specifier(root->childs[0]);
    return do_varcla(root->childs[1], tail, t, 0);
}

FieldList do_varcla(Node* root, FieldList tail1, Type tail2, int in_struct){
    if(root->child_no == 1){
        FieldList f = (FieldList)malloc(sizeof (struct FieldList_));
        strcpy(f->name, root->childs[0]->str + 4);
        f->type = tail2;
        f->tail = tail1;
        return f;
    }
    else{
        Type t =build_type(ARRAY, 0, tail2, root->childs[2]->ino, NULL, NULL);
        return do_varcla(root->childs[0], tail1, t, in_struct); 
    }
}

int do_fundec(Type returntype, Node* root){
    Entry e = lookup_func(symtab_h, root->childs[0]->str + 4);
    if(e != NULL && e->v.is_impl){
        if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 4 at Line %d: \n", root->line_no);
            return 0;
    }
    else{
        FieldList f = NULL;
        if(root->child_no >3){
            f = do_varlist(root->childs[2]);
            FieldList tmp = f;
            while(tmp){
                Entry e = lookup_symtab(symtab_h, tmp->name);
                e->u.is_param = 1;
                tmp = tmp->tail;
            }
        }
        if(e == NULL){
            insert_entry(FUNC, root->childs[0]->str + 4, returntype, f);
            symtab_h->v.is_impl = 1;
            return 1;
        }else{
            if(!is_same_field(f, e->v.paras) || !is_same_type(e->v.returntype, returntype)){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 19 at Line %d: \n", root->line_no);
                return 0;
            }else{
                e->v.paras = f;
                e->v.is_impl = 1;
                return 1;
            }
        }
    }
}

FieldList do_varlist(Node* root){
    FieldList f = NULL;
    if(root->child_no > 1){
        f = do_varlist(root->childs[2]);
    }
    return do_paramdec(root->childs[0], f);
}

FieldList do_paramdec(Node* root, FieldList tail){
    Type t =do_specifier(root->childs[0]);
    return do_vardec(root->childs[1], tail, t, 0);
}

void do_extdeclist(Type t, Node* root){
    do_vardec(root->childs[0], NULL, t, 0);
    if(root->child_no > 1) do_extdeclist(t, root->childs[2]);
}

Type do_specifier(Node* root){
    if(strcmp(root->childs[0]->str, "StructSpecifier") == 0){
        return do_struct_specifier(root->childs[0]);
    }else{
        if(root->childs[0]->str[6] == 'i') return build_type(0, 0, NULL, 0, NULL, NULL);
        else return build_type(0, 1, NULL, 0, NULL, NULL);
    }
}

FieldList reverse_list(FieldList List)
{
	FieldList s = NULL;
	FieldList i = NULL;	
	FieldList t = List;
	
	while (t != NULL)
	{
		i = t;
		t = t->tail;
		i->tail = s;
		s = i;
	}
    
	return i;
    
}

Type do_struct_specifier(Node* root){
    if(root->child_no == 2){
        Frame t = lookup_sttab(sttab_h, root->childs[1]->childs[0]->str + 4);
        
        if(t == NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 17 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
        else return t->type;
    }else{
        char name[40];
        if(root->childs[1] == NULL){
            strcpy(name, random_name);
            random_name[1]++;
        }else{
            strcpy(name, root->childs[1]->childs[0]->str + 4);
        }
        if(lookup_symtab(symtab_h, name)!= NULL || lookup_sttab(sttab_h, name)!=NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 16 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
        else {
            FieldList f = NULL;
            if(root->childs[3] != NULL){
                f = do_deflist(root->childs[3], NULL, 1);
                f = reverse_list(f);
            }
            Type t = build_type(2, 0, NULL, 0, f, name);
            insert_frame(name, t);
            return t;
        }
    }
}

FieldList do_deflist(Node* root, FieldList tail, int in_struct){
    FieldList f = do_def(root->childs[0], tail, in_struct);
    if(root->childs[1] != NULL)
        f = do_deflist(root->childs[1], f, in_struct);
    return f;
}

FieldList do_def(Node* root, FieldList tail, int in_struct){
    FieldList f = do_declist(do_specifier(root->childs[0]), root->childs[1], tail, in_struct);
    return f;
}

FieldList do_declist(Type t, Node* root, FieldList tail, int in_struct){
    FieldList f = do_dec(t, root->childs[0], tail, in_struct);
    
    if(root->child_no > 1){
       return do_declist(t, root->childs[2], f, in_struct);
    }
    else return f;
}

FieldList do_dec(Type t, Node* root, FieldList tail, int in_struct){
    tail = do_vardec(root->childs[0], tail, t, in_struct);
    if(root->child_no > 1 && in_struct){
        if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 15 at Line %d: \n", root->line_no);
    }else{
        if(root->child_no > 1){
            Type et = do_expr(root->childs[2]);
            if(tail == NULL || !is_same_type(et, tail->type)){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 5 at Line %d: \n", root->line_no);
            }
        }
    }
    return tail;
}

FieldList do_vardec(Node* root, FieldList tail1, Type tail2, int in_struct){
    if(root->child_no == 1){
        if(lookup_symtab(symtab_h, root->childs[0]->str + 4)!= NULL || lookup_sttab(sttab_h, root->childs[0]->str + 4)!= NULL){
            if(in_struct){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 15 at Line %d: \n", root->line_no);
            }
            else{
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 3 at Line %d: \n", root->line_no);
            } 
            return tail1;
        }else{
            FieldList f = (FieldList)malloc(sizeof (struct FieldList_));
            strcpy(f->name, root->childs[0]->str + 4);
            f->type = tail2;
            f->tail = tail1;
            insert_entry(VAR, f->name, tail2 , NULL);
            return f;
        }
    }else{
        Type t =build_type(ARRAY, 0, tail2, root->childs[2]->ino, NULL, NULL);
        return do_vardec(root->childs[0], tail1, t, in_struct); 
    }
}

void do_compst(Node* root, Type returntype){
    if(root->childs[1] != NULL)
        do_deflist(root->childs[1], NULL, 0);
    if(root->childs[2] != NULL){
        do_stmtlist(root->childs[2], returntype);
    }
}

void do_stmtlist(Node* root, Type returntype){
    do_stmt(root->childs[0], returntype);
    if(root->childs[1] != NULL) do_stmtlist(root->childs[1], returntype);
}

void do_stmt(Node* root, Type returntype){
    if(root->child_no == 1) do_compst(root->childs[0], returntype);
    else if(root->child_no == 2) do_expr(root->childs[0]);
    else if(root->child_no == 3){
        Type t = do_expr(root->childs[1]);
        if(is_same_type(t, returntype)){
            return;
        }else{
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 8 at Line %d: \n", root->line_no);
        }
    }else{
        Type t = do_expr(root->childs[2]);
        if(t->kind != BASIC || t->u.basic != 0){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 7 at Line %d: \n", root->line_no);
        }
        do_stmt(root->childs[4], returntype);
        if(root->child_no > 6)  
            do_stmt(root->childs[6], returntype);
    }
}

int is_same_type(Type a, Type b){
    if(a->kind != b->kind) return 0;
    else if(a->kind == BASIC) return a->u.basic == b->u.basic;
    else if(a->kind == ARRAY) return is_same_type(a->u.array.elem, b->u.array.elem);
    else if(a->kind == STRUCTURE) return strcmp(a->u.name, b->u.name) == 0;
    else return 0;
}

int is_same_paras(FieldList f, Node* root){
    if(!is_same_type(do_expr(root->childs[0]), f->type)) return 0;
    else if((root->child_no == 1 && f->tail != NULL) || (root->child_no !=1 && f->tail == NULL)) return 0;
    else if(root->child_no == 1 && f->tail == NULL) return 1;
    else return is_same_paras(f->tail, root->childs[2]);
}

Type find_field(FieldList f, char name[]){
    if(f == NULL) return NULL;
    else if(strcmp(f->name, name) == 0) return f->type;
    else return find_field(f->tail, name);
}

Type do_expr(Node* root){
    if(root->child_no == 1){
        if(root->childs[0]->type == 1){
            Entry e = lookup_symtab(symtab_h, root->childs[0]->str + 4);
            if(e == NULL){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 1 at Line %d: \n", root->line_no);
                return build_type(ERROR, 2, NULL, 0, NULL, NULL);
            }
            else return e->u.type;
        }
        else if(root->childs[0]->type == 2) return build_type(BASIC, 0, NULL, 0, NULL, NULL);
        else return build_type(BASIC, 1, NULL, 0, NULL, NULL);
    }
    else if(root->child_no == 3 && strcmp(root->childs[2]->str, "Exp") == 0){
        Type a = do_expr(root->childs[0]), b = do_expr(root->childs[2]);
        if(strcmp(root->childs[1]->str, "ASSIGNOP") == 0){
            if(root->childs[0]->can_left == 0){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 6 at Line %d: \n", root->line_no);
            }
            else if(!is_same_type(a,b)){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 5 at Line %d: \n", root->line_no);
            }
            return a;
        }
        else if(a->kind == BASIC && b->kind == BASIC && a->u.basic == b->u.basic){
            if(a->u.basic == 1 && (strcmp(root->childs[1]->str, "AND") == 0 || strcmp(root->childs[1]->str, "OR") == 0)){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 7 at Line %d: \n", root->line_no);
                return build_type(ERROR, 2, NULL, 0, NULL, NULL);
            }else if(a->u.basic == 1 && strcmp(root->childs[1]->str, "RELOP") == 0){
                return build_type(BASIC, 0, NULL, 0, NULL, NULL);
            }else return a;
        }else{
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 7 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
    }else if(root->child_no == 2){
        Type a = do_expr(root->childs[1]);
        if(a->kind == BASIC){
            if(a->u.basic == 1 && strcmp(root->childs[1]->str, "NOT") == 0){
                return build_type(ERROR, 2, NULL, 0, NULL, NULL);
            }
            else return a;
        }else{
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 7 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
    }else if(strcmp(root->childs[0]->str, "LP") == 0){
        return do_expr(root->childs[1]);
    }else if(strcmp(root->childs[1]->str, "LP") == 0){
        Entry e = lookup_symtab(symtab_h, root->childs[0]->str + 4);
        if(e != NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 11 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
        e = lookup_func(symtab_h, root->childs[0]->str + 4);
        if(e == NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 2 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else if(root->child_no == 3 && e->v.paras != NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 9 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else if(root->child_no == 4 && e->v.paras == NULL){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 9 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else if(root->child_no == 4 && !is_same_paras(e->v.paras, root->childs[2])){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 9 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else{
            return e->v.returntype;
        }
    }else if(root->child_no == 4){
        Type a = do_expr(root->childs[0]), b = do_expr(root->childs[2]);
        if(b->kind != BASIC || b->u.basic != 0){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 12 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else if(a->kind != ARRAY){
             if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 10 at Line %d: \n", root->line_no);
             return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }else return a->u.array.elem;
    }else{
        Type a = do_expr(root->childs[0]);
        if(a->kind != STRUCTURE){
            if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 13 at Line %d: \n", root->line_no);
            return build_type(ERROR, 2, NULL, 0, NULL, NULL);
        }
        else{
            Type t = find_field(a->u.structure, root->childs[2]->str + 4);
            if(t == NULL){
                if(!is_error[root->line_no]) is_error[root->line_no] = 1, printf("Error type 14 at Line %d: \n", root->line_no);
                return build_type(ERROR, 2, NULL, 0, NULL, NULL);
            }else{
                return t;
            }
        }
    }
}

void check(Entry head){
    if(head == NULL) return;
    if(head->kind == FUNC && !head->v.is_impl){
        if(!is_error[head->v.line_no]) is_error[head->v.line_no] = 1, printf("Error type 18 at Line %d: \n", head->v.line_no);
    }
    check(head->next);
}

void semantics(){
    FieldList f = (FieldList)malloc(sizeof (struct FieldList_));
    strcpy(f->name, "a");
    f->type = build_type(BASIC, 0, NULL, 0, NULL, NULL);
    f->tail = NULL;
    insert_entry(FUNC, "read", build_type(BASIC, 0, NULL, 0, NULL, NULL), NULL);
    symtab_h->v.is_impl = 1;
    insert_entry(FUNC, "write", build_type(BASIC, 0, NULL, 0, NULL, NULL), f);
    symtab_h->v.is_impl = 1;

    analyse(root);
    check(symtab_h);
}