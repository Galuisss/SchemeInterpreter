/**
 * @file parser.cpp
 * @brief Parsing implementation for Scheme syntax tree to expression tree conversion
 * 
 * This file implements the parsing logic that converts syntax trees into
 * expression trees that can be evaluated.
 * primitive operations, and function applications.
 */

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "value.hpp"
#include "expr.hpp"
#include <map>
#include <string>
#include <iostream>

#define mp make_pair
using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

/**
 * @brief Default parse method (should be overridden by subclasses)
 */
Expr Syntax::parse(Assoc &env) {
    return (*this)->parse(env);
    throw RuntimeError("Unimplemented parse method");
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse(Assoc &env) {
    //TODO: complete the rational parser
    return Expr(new RationalNum(numerator, denominator));
}

Expr SymbolSyntax::parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr StringSyntax::parse(Assoc &env) {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    if (stxs.empty()) {
        return Expr(new Quote(Syntax(new List())));
    }

    //TODO: check if the first element is a symbol
    //If not, use Apply function to package to a closure;
    //If so, find whether it's a variable or a keyword;
    SymbolSyntax *id = dynamic_cast<SymbolSyntax*>(stxs[0].get());
    
    if (id == nullptr) {
        //TODO: TO COMPLETE THE LOGIC
    }else{
    string op = id->s;
    if (find(op, env).get() != nullptr) {
        //TODO: TO COMPLETE THE PARAMETER PARSER LOGIC
    }
    if (primitives.count(op) != 0) {
        vector<Expr> parameters;

        for (auto it = stxs.begin() + 1; it != stxs.end(); it++) {
            parameters.push_back(it->parse(env));
        }
        ExprType op_type = primitives[op];

        switch (op_type)
        {
            // Arithmetic operations
            case E_PLUS:
            return Expr(new PlusVar(parameters));
            case E_MINUS:
            return Expr(new MinusVar(parameters));
            case E_MUL:
            return Expr(new MultVar(parameters));
            case E_DIV:
            return Expr(new DivVar(parameters));
            case E_MODULO:
            return parameters.size() == 2 ? Expr(new Modulo(parameters[0], parameters[1])): throw RuntimeError("Wrong number of arguments for expt");
            case E_EXPT:
            return parameters.size() == 2 ? Expr(new Expt(parameters[0], parameters[1])): throw RuntimeError("Wrong number of arguments for expt");
            // Comparison operations
            case E_LT:
            return Expr(new LessVar(parameters));
            case E_LE:
            return Expr(new LessEqVar(parameters));
            case E_EQ: 
            return Expr(new EqualVar(parameters));
            case E_GE: 
            return Expr(new GreaterEqVar(parameters));
            case E_GT: 
            return Expr(new GreaterVar(parameters));
            // Logic operations
            case E_NOT:
            return parameters.size() == 1 ? Expr(new Not(parameters[0])): throw RuntimeError("Wrong number of arguments for not");
            case E_AND:
            return Expr(new AndVar(parameters));
            case E_OR:
            return Expr(new OrVar(parameters));
            // List operations
            case E_CONS:
            case E_CAR:
            case E_CDR:
            case E_LIST:
            return Expr(new ListFunc(parameters));
            // Type predicates
            case E_EQQ:
            case E_BOOLQ:
            return parameters.size() == 1 ? Expr(new IsBoolean(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_INTQ:
            return parameters.size() == 1 ? Expr(new IsFixnum(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_NULLQ:
            return parameters.size() == 1 ? Expr(new IsNull(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_PAIRQ:
            return parameters.size() == 1 ? Expr(new IsPair(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_PROCQ:
            return parameters.size() == 1 ? Expr(new IsProcedure(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_SYMBOLQ:
            return parameters.size() == 1 ? Expr(new IsSymbol(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            case E_LISTQ: break;
            case E_STRINGQ:
            return parameters.size() == 1 ? Expr(new IsString(parameters[0])): throw RuntimeError("Wrong number of arguments for expt");
            default:
                break;
        }
    }

    if (reserved_words.count(op) != 0) {
    	switch (reserved_words[op]) {
            // Control flow constructs
            case E_BEGIN:
            case E_QUOTE:
            //Conditional
            case E_IF:
            case E_COND:
            // Variables and function definition
            //case E_VAR:
            //case E_APPLY:
            case E_LAMBDA:
            case E_DEFINE:
            // Binding constructs
            case E_LET:
            case E_LETREC:
            // Assignment
            case E_SET:
        	default:
            	throw RuntimeError("Unknown reserved word: " + op);
    	}
    }

    //default: use Apply to be an expression
    //TODO: TO COMPLETE THE PARSER LOGIC
}
}
