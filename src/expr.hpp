#ifndef EXPRESSION
#define EXPRESSION

/**
 * @file eclass RationalNum : public ExprBase {
public:
    int numerator;
    int denominator;
    RationalNum(int num, int den);
    virtual Expr eval(Assoc &) override;
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


// ================================================================================
//                             BASIC TYPES AND LITERALS
// ================================================================================

/**
 * @brief Integer literal expression
 * Represents fixed-point numbers (integers)
 */

struct ExprBase{
    ExprType e_type;
    ExprBase(ExprType);
    virtual Expr eval(Assoc &) = 0;
    inline virtual void show(std::ostream &) const {};
    virtual void showCdr(std::ostream &) const;
    virtual ~ExprBase() = default;
};

struct Expr {
    std::shared_ptr<ExprBase> ptr;
public:
    explicit Expr(ExprBase *);
    void show(std::ostream &) const;
    ExprBase* operator->() const;
    ExprBase& operator*();
    ExprBase* get() const;
};

inline std::ostream &operator<<(std::ostream &os, const Expr &v) {
    v->show(os);
    return os;
}
struct Assoc {
    std::shared_ptr<AssocList> ptr;
    Assoc(AssocList *);
    AssocList* operator->() const;
    AssocList& operator*();
    AssocList* get() const;
};

/**
 * @brief Association list node for variable bindings
 */
struct AssocList {
    std::string x;      ///< Variable name
    Expr v;            ///< Variable value
    Assoc next;         ///< Next binding in the chain
    AssocList(const std::string &, const Expr &, Assoc &);
};

// Environment operations
Assoc empty();
Assoc extend(const std::string&, const Expr &, Assoc &);
void modify(const std::string&, const Expr &, Assoc &);
Expr find(const std::string &, Assoc &);

struct self_evaluating : ExprBase{
    self_evaluating(ExprType);
    virtual Expr eval(Assoc &) override;
};
struct Fixnum : self_evaluating {
    int n;
    Fixnum(int);
    inline virtual void show(std::ostream &os) const override {
        os << n;
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr FixnumE(int n) {return Expr(new Fixnum(n));};

/**
 * @brief Rational number literal expression
 * Represents rational numbers as numerator/denominator
 */
struct RationalNum : self_evaluating {
    int numerator;
    int denominator;
    RationalNum(int num, int den);
    inline virtual void show(std::ostream &os) const override {
        if (denominator == 1) {
            os << numerator;
        } else {
            os << numerator << "/" << denominator;
        }
    };
    explicit RationalNum(const Fixnum& a);
    virtual Expr eval(Assoc &) override;
};
inline Expr RationalNumE(int n, int d) {return Expr(new RationalNum(n, d));};
/**
 * @brief String literal expression
 * Represents string Exprs
 */
struct StringExpr : self_evaluating {
    std::string s;
    StringExpr(const std::string &);
    inline virtual void show(std::ostream &os) const override {
        os << "\"" << s << "\"";
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr StringExprE(std::string s) {return Expr(new StringExpr(s));};
/**
 * @brief Boolean true literal
 */

struct Boolean : self_evaluating {
    Boolean(const bool &);
    bool b;
    inline virtual void show(std::ostream &os) const override {
        os << (b ? "#t" : "#f");
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr BooleanE(const bool &b) {return Expr(new Boolean(b));};
struct MakeVoid : self_evaluating {
    MakeVoid();
    inline virtual void show(std::ostream &os) const override {
        os << "#<void>";
    }
    virtual Expr eval(Assoc &) override;
};
inline Expr MakeVoidE() {return Expr(new MakeVoid());};

struct Exit : self_evaluating {
    Exit();
    inline virtual void show(std::ostream &) const override {};
    virtual Expr eval(Assoc &) override;
};
inline Expr ExitE() {return Expr(new Exit());};

struct NullExpr : self_evaluating {
    NullExpr();
    inline virtual void show(std::ostream &os) const override {
        os << "()";
    };
    inline virtual void showCdr(std::ostream &os) const override {
        os << ')';
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr NullExprE() {return Expr(new NullExpr());};

struct Pair : self_evaluating {
    Expr car;  ///< First element
    Expr cdr;  ///< Second element
    Pair(const Expr &, const Expr &);
    inline virtual void show(std::ostream &os) const override {
        os << '(' << car;
        cdr->showCdr(os);
    };
    inline virtual void showCdr(std::ostream &os) const override {
        os << ' ' << car;
        cdr->showCdr(os);
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr PairE(const Expr &car, const Expr &cdr) {return Expr(new Pair(car, cdr));};

struct Procedure : self_evaluating {
    std::vector<std::string> parameters;   ///< Parameter names
    Expr e;                                ///< Function body expression
    Assoc env;                             ///< Closure environment
    Procedure(const std::vector<std::string> &, const Expr &, const Assoc &);
    inline virtual void show(std::ostream &os) const override {
        os << "#<procedure>";
    };
    virtual Expr eval(Assoc &) override;
};
inline Expr ProcedureE(const std::vector<std::string> &vec, const Expr &e, const Assoc &env) {return Expr(new Procedure(vec, e, env));};

// ================================================================================
//                             BASIC ABSTRACT TYPES FOR PARAMETERS
// ================================================================================

struct Unary : ExprBase {
    Expr rand;
    Unary(ExprType, const Expr &);
    virtual Expr evalRator(const Expr &) = 0;
    virtual Expr eval(Assoc &) override;
};

struct Binary : ExprBase {
    Expr rand1;
    Expr rand2;
    Binary(ExprType, const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) = 0;
    virtual Expr eval(Assoc &) override;
};

struct Variadic : ExprBase {
    std::vector<Expr> rands;
    Variadic(ExprType, const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) = 0;
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                             ARITHMETIC OPERATIONS
// ================================================================================

struct Plus : Binary {
    Plus();
    Plus(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Minus : Binary {
    Minus(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Mult : Binary {
    Mult(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Div : Binary {
    Div(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Modulo : Binary {
    Modulo(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Expt : Binary {
    Expt(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct PlusVar : Variadic {
    PlusVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct MinusVar : Variadic {
    MinusVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct MultVar : Variadic {
    MultVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct DivVar : Variadic {
    DivVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

// ================================================================================
//                             COMPARISON OPERATIONS
// ================================================================================

struct Less : Binary {
    Less(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct LessEq : Binary {
    LessEq(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Equal : Binary {
    Equal(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct GreaterEq : Binary {
    GreaterEq(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Greater : Binary {
    Greater(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct LessVar : Variadic {
    LessVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct LessEqVar : Variadic {
    LessEqVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct EqualVar : Variadic {
    EqualVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct GreaterEqVar : Variadic {
    GreaterEqVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct GreaterVar : Variadic {
    GreaterVar(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

// ================================================================================
//                             LIST OPERATIONS
// ================================================================================

struct Cons : Binary {
    Cons(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct Car : Unary {
    Car(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct Cdr : Unary {
    Cdr(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct ListFunc : Variadic {
    ListFunc(const std::vector<Expr> &);
    virtual Expr evalRator(const std::vector<Expr> &) override;
};

struct SetCar : Binary {
    SetCar(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct SetCdr : Binary {
    SetCdr(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

// ================================================================================
//                             LOGIC OPERATIONS
// ================================================================================

struct Not : Unary {
    Not(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct AndVar : ExprBase {
    std::vector<Expr> rands;
    AndVar(const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;  
};

struct OrVar : ExprBase {
    std::vector<Expr> rands;
    OrVar(const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                             TYPE PREDICATES
// ================================================================================

struct IsEq : Binary {
    IsEq(const Expr &, const Expr &);
    virtual Expr evalRator(const Expr &, const Expr &) override;
};

struct IsBoolean : Unary {
    IsBoolean(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsFixnum : Unary {
    IsFixnum(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsNull : Unary {
    IsNull(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsPair : Unary {
    IsPair(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsProcedure : Unary {
    IsProcedure(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsSymbol : Unary {
    IsSymbol(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsList : Unary {
    IsList(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

struct IsString : Unary {
    IsString(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

// ================================================================================
//                             CONTROL FLOW CONSTRUCTS
// ================================================================================

struct Begin : ExprBase {
    std::vector<Expr> es;
    Begin(const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

struct Quote : ExprBase {
    //Syntax s;
    Expr ex;
    Quote(const Expr &);
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                             CONDITIONALS
// ================================================================================

struct If : ExprBase {
  Expr cond;
  Expr conseq;
  Expr alter;
  If(const Expr &, const Expr &, const Expr &);
  virtual Expr eval(Assoc &) override;
};

struct Cond : ExprBase {
    std::vector<Expr> clauses;
    Cond(const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                             VARIABLE AND FUNCITION DEFINITION
// ================================================================================

struct Var : ExprBase {
    std::string x;
    Var(const std::string &);
    virtual void show(std::ostream &os) const override {
        os << x;
    }
    virtual Expr eval(Assoc &) override;
};
struct SList : ExprBase {
    std::vector<Expr> terms;
    SList(const std::vector<Expr> t);
    virtual void show(std::ostream &os) const override {
        os << '(';
        for (auto t : terms) {
            t->show(os);
        }
        os << ')';
    } 
    virtual Expr eval(Assoc &) override;
};
struct Apply : ExprBase {
    Expr rator;
    std::vector<Expr> rand;
    Apply(const Expr &, const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

struct Lambda : ExprBase {
    std::vector<std::string> x;
    Expr e;
    Lambda(const std::vector<std::string> &, const Expr &);
    virtual Expr eval(Assoc &) override;
};

struct Define : ExprBase {
    std::string var;
    Expr e;
    Define(const std::string &, const Expr &);
    virtual Expr eval(Assoc &) override;
};

struct Define_f : ExprBase {
    std::string var;
    std::vector<std::string> x;
    Expr e;
    Define_f(const std::string &, std::vector<std::string>&, const Expr &);
    virtual Expr eval(Assoc &) override;
};

struct Primitive : self_evaluating {
    ExprType type;
    Primitive(ExprType);
    virtual Expr eval(Assoc &) override;
    inline virtual void show(std::ostream &os) const override {
        os << "#<procedure>";
    }
};
inline Expr PrimitiveE(ExprType et) {return Expr(new Primitive(et));};

struct SpecialForm : self_evaluating {
    ExprType type;
    SpecialForm(ExprType);
    virtual Expr eval(Assoc &) override;
};
inline Expr SpecialFormE(ExprType et) {return Expr(new SpecialForm(et));};
// ================================================================================
//                             BINDING CONSTRUCTS
// ================================================================================

struct Let : ExprBase {
    std::vector<std::pair<std::string, Expr>> bind;
    std::vector<Expr> body;
    Let(const std::vector<std::pair<std::string, Expr>> &, const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

struct Letrec : ExprBase {
    std::vector<std::pair<std::string, Expr>> bind;
    std::vector<Expr> body;
    Letrec(const std::vector<std::pair<std::string, Expr>> &, const std::vector<Expr> &);
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                             ASSIGNMENT
// ================================================================================

struct Set : ExprBase {
    std::string var;
    Expr e;
    Set(const std::string &, const Expr &);
    virtual Expr eval(Assoc &) override;
};

// ================================================================================
//                              I/O OPERATIONS
// ================================================================================

struct Display : Unary {
    Display(const Expr &);
    virtual Expr evalRator(const Expr &) override;
};

#endif
