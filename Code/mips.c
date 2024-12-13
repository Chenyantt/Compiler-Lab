#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"

FILE *f;

char funcs[300];

int var_cnt,off;

struct PSI{
    char str[100];
    int off;
}vars[10000];

int check_var(char* v){
    for(int i=0;i<var_cnt;++i){
        if(strcmp(vars[i].str, v) == 0) 
            return 1;
    }
    return 0;
}

void add_var(char* v,int sz){
    strcpy(vars[var_cnt].str, v);
    off -= sz;
    vars[var_cnt++].off = off;
}

void deal_line(char* line){
    char t1[100],t2[100];
    if(strncmp(line, "DEC", 3) == 0){
        int sz;
        sscanf(line, "DEC %s %d",t1, &sz);
        add_var(t1, sz);
    }else if(strncmp(line, "READ", 4) == 0){
        sscanf(line, "READ %s",t1);
        if(!check_var(t1)) add_var(t1, 4);
    }else if(sscanf(line, "%s := %s", t1, t2) == 2 && *t1 != '*'){
        if(!check_var(t1)) add_var(t1, 4);
    }
}

int line_cnt;
char lines[1000][100];

int find_var(char* v){
    for(int i=0;i<var_cnt;++i){
        if(strcmp(vars[i].str, v) == 0) 
            return vars[i].off;
    }
    return 0;
}

void m2r(char* r, char* str){
    int off;
    if(*str == '#'){
        fprintf(f, "li %s,%s\n",r,str+1);
    }
    else if(*str == '*'){
        off = find_var(str + 1);
        fprintf(f, "lw %s,%d($fp)\n",r,off);
        fprintf(f, "lw %s,0(%s)\n",r,r);
    }else if(*str == '&'){
        off = find_var(str + 1);
        fprintf(f, "la %s,%d($fp)\n",r,off);
    }else{
        off = find_var(str);
        fprintf(f, "lw %s,%d($fp)\n",r,off);
    }
}

void deal_func(){
    char t1[100],t2[100],t3[100];
    for(int i=0;i<line_cnt;++i){
        if(strncmp("DEC", lines[i], 3) == 0){
            continue;
        }
        if(strncmp("LABEL", lines[i], 5) == 0){
            sscanf(lines[i], "LABEL %s :",t1);
            fprintf(f, "%s:\n", t1);
        }else if(strncmp("GOTO", lines[i], 4) == 0){
            sscanf(lines[i], "GOTO %s",t1);
            fprintf(f, "j %s\n", t1);
            
        }else if(strncmp("READ", lines[i], 4) == 0){
            sscanf(lines[i], "READ %s", t1);
            fprintf(f, "addi $sp,$sp,-4\n");
            fprintf(f, "sw $ra,0($sp)\n");
            fprintf(f, "jal _read\n");
            fprintf(f, "lw $ra,0($sp)\n");
            fprintf(f, "addi $sp,$sp,4\n");
            fprintf(f, "sw $v0, %d($fp)\n",find_var(t1));
        }else if(strncmp("WRITE", lines[i], 5) == 0){
            sscanf(lines[i], "WRITE %s", t1);
            fprintf(f, "lw $a0, %d($fp)\n",find_var(t1));
            fprintf(f, "addi $sp,$sp,-4\n");
            fprintf(f, "sw $ra,0($sp)\n");
            fprintf(f, "jal _write\n");
            fprintf(f, "lw $ra,0($sp)\n");
            fprintf(f, "addi $sp,$sp,4\n");
        }
        else if(strncmp("RETURN", lines[i], 6) == 0){
            sscanf(lines[i], "RETURN %s",t1);
            m2r("$v0", t1);
            fprintf(f, "move $sp,$fp\n");
            fprintf(f, "lw $fp,0($sp)\n");
            fprintf(f, "addi $sp,$sp,4\n");
            fprintf(f, "jr $ra\n");
        }else if(strncmp("ARG", lines[i], 3) == 0){
            sscanf(lines[i], "ARG %s",t1);
            m2r("$t1", t1);
            fprintf(f, "addi $sp,$sp,-4\n");
            fprintf(f, "sw $t1, 0($sp)\n");
        }else if(strstr(lines[i],"CALL")){
            sscanf(lines[i], "%s := CALL %s", t1, t2);
            fprintf(f, "addi $sp,$sp,-4\n");
            fprintf(f, "sw $ra,0($sp)\n");
            if(strcmp(t2,"main")==0) fprintf(f, "jal %s\n", t2);
            else fprintf(f, "jal _%s\n", t2);
            fprintf(f, "lw $ra,0($sp)\n");
            fprintf(f, "addi $sp,$sp,4\n");
            if(strcmp("(null)", t1)){
                fprintf(f, "sw $v0, %d($fp)\n",find_var(t1));
            }
        }else if(strncmp("IF", lines[i], 2) == 0){
            if(strstr(lines[i], "==")){
                sscanf(lines[i],"IF %s == %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "beq $t1,$t2,%s\n", t3);
            }else if(strstr(lines[i], "!=")){
                sscanf(lines[i],"IF %s != %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "bne $t1,$t2,%s\n", t3);
            }else if(strstr(lines[i], "<=")){
                sscanf(lines[i],"IF %s <= %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "ble $t1,$t2,%s\n", t3);
            }else if(strstr(lines[i], ">=")){
                sscanf(lines[i],"IF %s >= %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "bge $t1,$t2,%s\n", t3);
            }else if(strstr(lines[i], "<")){
                sscanf(lines[i],"IF %s < %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "blt $t1,$t2,%s\n", t3);
            }else{
                sscanf(lines[i],"IF %s > %s GOTO %s", t1, t2, t3);
                m2r("$t1", t1), m2r("$t2", t2);
                fprintf(f, "bgt $t1,$t2,%s\n", t3);
            }
        }else{
            if(*lines[i] == '*'){
                sscanf(lines[i],"%s := %s", t1, t2);
                m2r("$t2", t2);
                fprintf(f, "lw $t1, %d($fp)\n",find_var(t1 + 1));
                fprintf(f, "sw $t2, 0($t1)\n");
            }else if(strstr(lines[i], ":= *")){
                sscanf(lines[i],"%s := %s", t1, t2);
                m2r("$t2", t2);
                fprintf(f, "sw $t2, %d($fp)\n",find_var(t1));
            }
            else if(strstr(lines[i], "+")){
                sscanf(lines[i],"%s := %s + %s", t1, t2, t3);
                m2r("$t2", t2), m2r("$t3", t3);
                fprintf(f, "add $t1,$t2,$t3\n");
                fprintf(f, "sw $t1, %d($fp)\n",find_var(t1));
            }else if(strstr(lines[i], "-")){
                sscanf(lines[i],"%s := %s - %s", t1, t2, t3);
                m2r("$t2", t2), m2r("$t3", t3);
                fprintf(f, "sub $t1,$t2,$t3\n");
                fprintf(f, "sw $t1, %d($fp)\n",find_var(t1));
            }else if(strstr(lines[i], "*")){
                sscanf(lines[i],"%s := %s * %s", t1, t2, t3);
                m2r("$t2", t2), m2r("$t3", t3);
                fprintf(f, "mul $t1,$t2,$t3\n");
                fprintf(f, "sw $t1, %d($fp)\n",find_var(t1));
            }else if(strstr(lines[i], "/")){
                sscanf(lines[i],"%s := %s / %s", t1, t2, t3);
                m2r("$t2", t2), m2r("$t3", t3);
                fprintf(f, "div $t2,$t3\n");
                fprintf(f, "mflo $t1\n");
                fprintf(f, "sw $t1, %d($fp)\n",find_var(t1));
            }else{
                sscanf(lines[i],"%s := %s", t1, t2);
                m2r("$t2", t2);
                fprintf(f, "sw $t2, %d($fp)\n",find_var(t1));
            }
        }
    }
}

static const char start[] = 
  ".data\n"
  "_prompt: .asciiz \"Enter an integer:\"\n"
  "_ret: .asciiz \"\\n\"\n"
  ".globl main\n"
  ".text\n"
  "_read:\n"
  "li $v0, 4\n"
  "la $a0, _prompt\n"
  "syscall\n"
  "li $v0, 5\n"
  "syscall\n"
  "jr $ra\n"
  "_write:\n"
  "li $v0, 1\n"
  "syscall\n"
  "li $v0, 4\n"
  "la $a0, _ret\n"
  "syscall\n"
  "move $v0, $0\n"
  "jr $ra\n";

void trans_mips(FILE* in, FILE* out){
    f = out;
    fprintf(f, "%s", start);
    char line[301],tmp[301];
    fgets(line, 300, in);
    int flag = 1;
    do{
        var_cnt = 0,off = 0,line_cnt = 0;
        sscanf(line,"FUNCTION %s :",tmp);
        if(strcmp(tmp,"main") == 0) fprintf(f, "%s:\n",tmp);
        else fprintf(f, "_%s:\n",tmp);
        for(int i = 0;;i++){
            fgets(line, 300, in);
            if(strncmp(line, "PARAM", 5)) break;
            sscanf(line,"PARAM %s",tmp);
            strcpy(vars[var_cnt].str, tmp);
            vars[var_cnt++].off = 4 * i + 8;
        }
        do{
            if(*line == '\n'){
                flag = 0;
                break;
            }
            if(strncmp(line, "FUNCTION ", 9) == 0) break;
            deal_line(line);
            strcpy(lines[line_cnt++], line);
            if(!fgets(line, 300, in)) {
                flag = 0;
                break;
            }
        }while(1);
        fprintf(f, "addi $sp,$sp,-4\n");
        fprintf(f, "sw $fp,0($sp)\n");
        fprintf(f, "move $fp,$sp\n");
        fprintf(f, "addi $sp,$sp,%d\n",off);
        deal_func();
    }while(flag);
}