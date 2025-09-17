#ifndef EXPRESSION
#define EXPRESSION

/**
 * @file eclass RationalNum : public ExprBase {
public:
    int numerator;
    int denominator;
    RationalNum(int num, int den);
    virtual Value eval(Assoc &) override;
};p
 * @brief Expression structures for the Scheme interpreter
 * @author luke36
 * 
 * This file defines all expression types used in the Scheme interpreter.
 * Structures are organized according to ExprType enumeration order from
 * Def.hpp for consistency and maintainability.
 */

#include "Def.hpp"
#include "syntax.hpp"
#include <memory>
#include <cstring>
#include <vector>

struct ExprBase{
    ExprType e_type;
    ExprBase(ExprType);
    virtual Value eval(Assoc &) = 0;
    virtual ~ExprBase() = default;
};

class Expr {
    std::shared_ptr<ExprBase> ptr;
public:
    Expr(ExprBase *);
    ExprBase* operator->() const;
    ExprBase& operator*();
    ExprBase* get() const;
};

// ================================================================================
//                             BASIC TYPES AND LITERALS
// ================================================================================

/**
 * @brief Integer literal expression
 * Represents fixed-point numbers (integers)
 */
struct Fixnum : ExprBase {
  int n;
  Fixnum(int);
  virtual Value eval(Assoc &) override;
};

/**
 * @brief Rational number literal expression
 * Represents rational numbers as numerator/denominator
 */
struct RationalNum : ExprBase {
  int numerator;
  int denominator;
  RationalNum(int num, int den);
  virtual Value eval(Assoc &) override;
};

/**
 * @brief String literal expression
 * Represents string values
 */
struct StringExpr : ExprBase {
  std::string s;
  StringExpr(const std::string &);
  virtual Value eval(Assoc &) override;
};

/**
 * @brief Boolean true literal
 */
struct True : ExprBase {
  True();
  virtual Value eval(Assoc &) override;
};

/**
 * @brief Boolean false literal  
 */
struct False : ExprBase {
  False();
  virtual Value eval(Assoc &) override;
};

struct MakeVoid : ExprBase {
    MakeVoid();
    virtual Value eval(Assoc &) override;
};

struct Exit : ExprBase {
    Exit();
    virtual Value eval(Assoc &) override;
};

struct NullExpr : ExprBase {
    NullExpr();
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             BASIC ABSTRACT TYPES FOR PARAMETERS
// ================================================================================

struct Unary : ExprBase {
    Expr rand;
    Unary(ExprType, const Expr &);
    virtual Value evalRator(const Value &) = 0;
    virtual Value eval(Assoc &) override;
};

struct Binary : ExprBase {
    Expr rand1;
    Expr rand2;
    Binary(ExprType, const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) = 0;
    virtual Value eval(Assoc &) override;
};

struct Variadic : ExprBase {
    std::vector<Expr> rands;
    Variadic(ExprType, const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) = 0;
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             ARITHMETIC OPERATIONS
// ================================================================================

struct Plus : Binary {
    Plus();
    Plus(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Minus : Binary {
    Minus(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Mult : Binary {
    Mult(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Div : Binary {
    Div(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Modulo : Binary {
    Modulo(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Expt : Binary {
    Expt(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct PlusVar : Variadic {
    PlusVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct MinusVar : Variadic {
    MinusVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct MultVar : Variadic {
    MultVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct DivVar : Variadic {
    DivVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

// ================================================================================
//                             COMPARISON OPERATIONS
// ================================================================================

struct Less : Binary {
    Less(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct LessEq : Binary {
    LessEq(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Equal : Binary {
    Equal(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct GreaterEq : Binary {
    GreaterEq(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Greater : Binary {
    Greater(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct LessVar : Variadic {
    LessVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct LessEqVar : Variadic {
    LessEqVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct EqualVar : Variadic {
    EqualVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct GreaterEqVar : Variadic {
    GreaterEqVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct GreaterVar : Variadic {
    GreaterVar(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

// ================================================================================
//                             LIST OPERATIONS
// ================================================================================

struct Cons : Binary {
    Cons(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct Car : Unary {
    Car(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct Cdr : Unary {
    Cdr(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct ListFunc : Variadic {
    ListFunc(const std::vector<Expr> &);
    virtual Value evalRator(const std::vector<Value> &) override;
};

struct SetCar : Binary {
    SetCar(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct SetCdr : Binary {
    SetCdr(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

// ================================================================================
//                             LOGIC OPERATIONS
// ================================================================================

struct Not : Unary {
    Not(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct AndVar : ExprBase {
    std::vector<Expr> rands;
    AndVar(const std::vector<Expr> &);
    virtual Value eval(Assoc &) override;  
};

struct OrVar : ExprBase {
    std::vector<Expr> rands;
    OrVar(const std::vector<Expr> &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             TYPE PREDICATES
// ================================================================================

struct IsEq : Binary {
    IsEq(const Expr &, const Expr &);
    virtual Value evalRator(const Value &, const Value &) override;
};

struct IsBoolean : Unary {
    IsBoolean(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsFixnum : Unary {
    IsFixnum(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsNull : Unary {
    IsNull(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsPair : Unary {
    IsPair(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsProcedure : Unary {
    IsProcedure(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsSymbol : Unary {
    IsSymbol(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsList : Unary {
    IsList(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct IsString : Unary {
    IsString(const Expr &);
    virtual Value evalRator(const Value &) override;
};

// ================================================================================
//                             CONTROL FLOW CONSTRUCTS
// ================================================================================

struct Begin : ExprBase {
    std::vector<Expr> es;
    Begin(const std::vector<Expr> &);
    virtual Value eval(Assoc &) override;
};

struct Quote : ExprBase {
    //Syntax s;
    Expr ex;
    Quote(const Expr &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             CONDITIONALS
// ================================================================================

struct If : ExprBase {
  Expr cond;
  Expr conseq;
  Expr alter;
  If(const Expr &, const Expr &, const Expr &);
  virtual Value eval(Assoc &) override;
};

struct Cond : ExprBase {
    std::vector<std::vector<Expr>> clauses;
    Cond(const std::vector<std::vector<Expr>> &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             VARIABLE AND FUNCITION DEFINITION
// ================================================================================

struct Var : ExprBase {
    std::string x;
    Var(const std::string &);
    virtual Value eval(Assoc &) override;
};

struct Apply : ExprBase {
    Expr rator;
    std::vector<Expr> rand;
    Apply(const Expr &, const std::vector<Expr> &);
    virtual Value eval(Assoc &) override;
};

struct Lambda : ExprBase {
    std::vector<std::string> x;
    Expr e;
    Lambda(const std::vector<std::string> &, const Expr &);
    virtual Value eval(Assoc &) override;
};

struct Define : ExprBase {
    std::string var;
    Expr e;
    Define(const std::string &, const Expr &);
    virtual Value eval(Assoc &) override;
};

struct Define_f : ExprBase {
    std::string var;
    std::vector<std::string> x;
    Expr e;
    Define_f(const std::string &, std::vector<std::string>&, const Expr &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             BINDING CONSTRUCTS
// ================================================================================

struct Let : ExprBase {
    std::vector<std::pair<std::string, Expr>> bind;
    Expr body;
    Let(const std::vector<std::pair<std::string, Expr>> &, const Expr &);
    virtual Value eval(Assoc &) override;
};

struct Letrec : ExprBase {
    std::vector<std::pair<std::string, Expr>> bind;
    Expr body;
    Letrec(const std::vector<std::pair<std::string, Expr>> &, const Expr &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                             ASSIGNMENT
// ================================================================================

struct Set : ExprBase {
    std::string var;
    Expr e;
    Set(const std::string &, const Expr &);
    virtual Value eval(Assoc &) override;
};

// ================================================================================
//                              I/O OPERATIONS
// ================================================================================

struct Display : Unary {
    Display(const Expr &);
    virtual Value evalRator(const Value &) override;
};

struct Quoted_Symbol : ExprBase {
    std::string var;
    Quoted_Symbol(const std::string &s);
    virtual Value eval(Assoc &) override;
};
#endif
