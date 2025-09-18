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
#include "expr.hpp"
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Expr Syntax::parse() {
    return (*this)->parse();
}

Expr Number::parse() {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse() {
    return Expr(new RationalNum(numerator, denominator));
}

Expr SymbolSyntax::parse() {
    return Expr(new Var(s));
}

Expr StringSyntax::parse() {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse() {
    return Expr(new Boolean(true));
}

Expr FalseSyntax::parse() {
    return Expr(new Boolean(false));
}

Expr List::parse() {
    std::vector<Expr> exprs;
    std::transform(stxs.begin(), stxs.end(), std::back_inserter(exprs),[](Syntax s) {return s->parse();});
    return Expr(new SList(exprs));
}

