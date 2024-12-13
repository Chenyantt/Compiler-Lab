#include <stdio.h>
#include "ast.h"
#include "semantics.h"
#include "ir.h"
#include "mips.h"

extern FILE* yyin;

 int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r"), *out = fopen("out.ir", "w"), *res = fopen(argv[2], "w");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    if(!out)
    {
        perror(argv[2]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    semantics();
    if(trans_ir(out)){
        fclose(out);
        remove(argv[2]);
        return 0;
    }
    fclose(out);
    out = fopen("out.ir","r");
    trans_mips(out, res);
    return 0;
}