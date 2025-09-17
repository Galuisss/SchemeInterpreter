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
#include <numeric>

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

Expr Syntax::q_parse() {
    return (*this)->q_parse();
    throw RuntimeError("Unimplemented parse method");
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr Number::q_parse() {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse(Assoc &env) {
    return Expr(new RationalNum(numerator, denominator));
}

Expr RationalSyntax::q_parse() {
    return Expr(new RationalNum(numerator, denominator));
}

Expr SymbolSyntax::parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr SymbolSyntax::q_parse() {
    return Expr(new Quoted_Symbol(s));
}

Expr StringSyntax::parse(Assoc &env) {
    return Expr(new StringExpr(s));
}

Expr StringSyntax::q_parse() {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr TrueSyntax::q_parse() {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr FalseSyntax::q_parse() {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    if (stxs.empty()) {
        return Expr(new Quote(Expr(new NullExpr())));
    }

    //TODO: check if the first element is a symbol
    //If not, use Apply function to package to a closure;
    //If so, find whether it's a variable or a keyword;
    SymbolSyntax *id = dynamic_cast<SymbolSyntax*>(stxs[0].get());
    
    if (id == nullptr) {
        //TODO: TO COMPLETE THE LOGIC
        throw(RuntimeError("No input"));
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
            return parameters.size() >= 1 ? Expr(new MinusVar(parameters)): throw RuntimeError("Wrong number of arguments for -");
            case E_MUL:
            return Expr(new MultVar(parameters));
            case E_DIV:
            return parameters.size() >= 1 ? Expr(new DivVar(parameters)): throw RuntimeError("Wrong number of arguments for /");
            case E_MODULO:
            return parameters.size() == 2 ? Expr(new Modulo(parameters[0], parameters[1])): throw RuntimeError("Wrong number of arguments for modulo");
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
            return parameters.size() == 2 ? Expr(new Cons(parameters[0], parameters[1])): throw RuntimeError("Wrong number of arguments for cons");
            case E_CAR:
            return parameters.size() == 1 ? Expr(new Car(parameters[0])) : throw RuntimeError("Wrong number of arguments for car");
            case E_CDR:
            return parameters.size() == 1 ? Expr(new Cdr(parameters[0])) : throw RuntimeError("Wrong number of arguments for cdr");
            case E_LIST:
            return Expr(new ListFunc(parameters));
            // Type predicates
            case E_EQQ:
            case E_BOOLQ:
            return parameters.size() == 1 ? Expr(new IsBoolean(parameters[0])): throw RuntimeError("Wrong number of arguments for boolean?");
            case E_INTQ:
            return parameters.size() == 1 ? Expr(new IsFixnum(parameters[0])): throw RuntimeError("Wrong number of arguments for number??");
            case E_NULLQ:
            return parameters.size() == 1 ? Expr(new IsNull(parameters[0])): throw RuntimeError("Wrong number of arguments for null?");
            case E_PAIRQ:
            return parameters.size() == 1 ? Expr(new IsPair(parameters[0])): throw RuntimeError("Wrong number of arguments for pair?");
            case E_PROCQ:
            return parameters.size() == 1 ? Expr(new IsProcedure(parameters[0])): throw RuntimeError("Wrong number of arguments for procedure?");
            case E_SYMBOLQ:
            return parameters.size() == 1 ? Expr(new IsSymbol(parameters[0])): throw RuntimeError("Wrong number of arguments for symbol?");
            case E_LISTQ: 
            return parameters.size() == 1 ? Expr(new IsList(parameters[0])): throw RuntimeError("Wrong number of arguments for list?");
            case E_STRINGQ:
            return parameters.size() == 1 ? Expr(new IsString(parameters[0])): throw RuntimeError("Wrong number of arguments for string?");
            default:
                break;
        }
    }

    if (reserved_words.count(op) != 0) {
    	switch (reserved_words[op]) { 
            // Control flow constructs
            case E_BEGIN:
            break;
            case E_QUOTE:
            return stxs.size() == 2 ? Expr(new Quote(stxs[1]->q_parse())): throw RuntimeError("Wrong number of arguments for quote");
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


Expr List::q_parse() {

    if (stxs.size() >= 3) {
        auto s = *(stxs.rbegin() + 1);
        auto ss = static_cast<SymbolSyntax*>(s.get());
        if (ss != nullptr && ss->s == ".") {
            Expr ex = (*stxs.rbegin())->q_parse();
            return std::accumulate(
            (stxs.rbegin() + 2), stxs.rend(), ex,
            [](Expr tail, const Syntax& x) {
            return Expr(new Cons(x->q_parse(), tail));
        }
        );
        }
    }

    Expr ex = Expr(new NullExpr());
    return std::accumulate(
        stxs.rbegin(), stxs.rend(), ex,
        [](Expr tail, const Syntax& x) {
            return Expr(new Cons(x->q_parse(), tail));
        }
    );
}