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
#include <algorithm>

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
    /*
    if (stxs.empty()) {
        //std::cout << "DEBUG: stxs is empty\n";
        return Expr(new Quote(Expr(new NullExpr())));
    }
    */

    std::vector<Expr> exprs;
    std::transform(stxs.begin(), stxs.end(), std::back_inserter(exprs),[&env](Syntax s) {return s->parse(env);});
    return Expr(new SList(exprs));
}


Expr List::q_parse() {

    if (stxs.size() >= 3) {
        auto s = *(stxs.rbegin() + 1);
        auto ss = dynamic_cast<SymbolSyntax*>(s.get());
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