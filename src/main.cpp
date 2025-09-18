#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include <sstream>
#include <iostream>
#include <map>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

void REPL(){
    // read - evaluation - print loop
    EnvPtr global_env = std::make_shared<Env>();

    while (1){
        #ifndef ONLINE_JUDGE
            std::cout << "scm> ";
        #endif
        Syntax stx = readSyntax(std :: cin); // read
        try{
            Expr expr = stx -> parse(); // parse

            Expr val = expr -> eval(global_env);
            if (val.ptr == nullptr)
            {   
                puts("");
                continue;
            }
            if (val->e_type == E_EXIT)
            {
                break;
            }
            val.show(std :: cout); // value print
        }
        catch (const RuntimeError &RE){
            //std :: cout << "DEBUG: " << RE.message() << std::endl;
            std :: cout << "RuntimeError";
        }
        puts("");
    }
}


int main(int argc, char *argv[]) {
    REPL();
    return 0;
}
