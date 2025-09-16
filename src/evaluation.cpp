/**
 * @file evaluation.cpp
 * @brief Expression evaluation implementation for the Scheme interpreter
 * @author luke36
 * 
 * This file implements evaluation methods for all expression types in the Scheme
 * interpreter. Functions are organized according to ExprType enumeration order
 * from Def.hpp for consistency and maintainability.
 */

#include "value.hpp"
#include "expr.hpp" 
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>
#include <climits>
#include <numeric>
#include <algorithm>
#include <functional>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Value Fixnum::eval(Assoc &e) { // evaluation of a fixnum
    return IntegerV(n);
}

Value RationalNum::eval(Assoc &e) { // evaluation of a rational number
    return RationalV(numerator, denominator);
}

Value StringExpr::eval(Assoc &e) { // evaluation of a string
    return StringV(s);
}

Value True::eval(Assoc &e) { // evaluation of #t
    return BooleanV(true);
}

Value False::eval(Assoc &e) { // evaluation of #f
    return BooleanV(false);
}

Value MakeVoid::eval(Assoc &e) { // (void)
    return VoidV();
}

Value Exit::eval(Assoc &e) { // (exit)
    return TerminateV();
}

Value Unary::eval(Assoc &e) { // evaluation of single-operator primitive
    return evalRator(rand->eval(e));
}

Value Binary::eval(Assoc &e) { // evaluation of two-operators primitive
    return evalRator(rand1->eval(e), rand2->eval(e));
}

Value Variadic::eval(Assoc &e) { // evaluation of multi-operator primitive
    // TODO: TO COMPLETE THE VARIADIC CLASS
    std::vector<Value> results;
    results.reserve(rands.size());
    std::transform(rands.begin(), rands.end(), std::back_inserter(results), [&e](const Expr &x){return x->eval(e);});
    return evalRator(results);
}

Value Var::eval(Assoc &e) { // evaluation of variable
    // TODO: TO identify the invalid variable
    // We request all valid variable just need to be a symbol,you should promise:
    //The first character of a variable name cannot be a digit or any character from the set: {.@}
    //If a string can be recognized as a number, it will be prioritized as a number. For example: 1, -1, +123, .123, +124., 1e-3
    //Variable names can overlap with primitives and reserve_words
    //Variable names can contain any non-whitespace characters except #, ', ", `, but the first character cannot be a digit
    //When a variable is not defined in the current scope, your interpreter should output RuntimeError
    
    Value matched_value = find(x, e);
    if (matched_value.get() == nullptr) {
        if (primitives.count(x)) {
            Expr exp = nullptr;
            switch (primitives[x]) {
                case E_PLUS: { exp = (new Plus(new Var("parm1"), new Var("parm2"))); break; }
                case E_MODULO: { exp = (new Modulo(new Var("parm1"), new Var("parm2"))); break; }
                case E_VOID: { exp = (new MakeVoid()); break; }
                case E_EQQ: { exp = (new IsEq(new Var("parm1"), new Var("parm2"))); break; }
                case E_BOOLQ: { exp = (new IsBoolean(new Var("parm"))); break; }
                case E_INTQ: { exp = (new IsFixnum(new Var("parm"))); break; }
                case E_NULLQ: { exp = (new IsNull(new Var("parm"))); break; }
                case E_PAIRQ: { exp = (new IsPair(new Var("parm"))); break; }
                case E_PROCQ: { exp = (new IsProcedure(new Var("parm"))); break; }
                case E_SYMBOLQ: { exp = (new IsSymbol(new Var("parm"))); break; }
                case E_STRINGQ: { exp = (new IsString(new Var("parm"))); break; }
                case E_EXPT: { exp = (new Expt(new Var("parm1"), new Var("parm2"))); break; }
                case E_DISPLAY: { exp = (new Display(new Var("parm"))); break; }
                case E_EXIT: { exp = (new Exit()); break; }
            }
            std::vector<std::string> parameters_;
            //TODO: to PASS THE parameters_ correctly;
            //COMPLETE THE CODE WITH THE HINT
            return ProcedureV(parameters_, exp, e);
        } else {
            throw(RuntimeError("undefined variable"));
        }
    }
    return matched_value;
}

bool isInt(const Value &v) {
    return v->v_type == V_INT;
}

bool isRat(const Value &v) {
    return v->v_type == V_RATIONAL;
}

bool isNum(const Value &v) {
    return isInt(v) || isRat(v);
}
Rational toRational(const Value& v) {
    switch (v->v_type) {
    case V_INT:
        return Rational(*static_cast<Integer*>(v.get()));
    case V_RATIONAL:
        return *static_cast<Rational*>(v.get());
    default:
        throw std::runtime_error("Not a number");
    }
}

Value H_Plus(const Value &rand1, const Value &rand2) {
    if (isNum(rand1) && isNum(rand2)) {
        Rational addend1 = toRational(rand1);
        Rational addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * denominator2 + denominator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        Rational ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return IntegerV(ans.numerator);
        }
        return RationalV(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Value Plus::evalRator(const Value &rand1, const Value &rand2) { // +
    //TODO: To complete the addition logic
    return H_Plus(rand1, rand2);
}

Value H_Minus(const Value &rand1, const Value &rand2) { // -
    //TODO: To complete the substraction logic
    if (isNum(rand1) && isNum(rand2)) {
        Rational addend1 = toRational(rand1);
        Rational addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * denominator2 - denominator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        Rational ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return IntegerV(ans.numerator);
        }
        return RationalV(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Value Minus::evalRator(const Value &rand1, const Value &rand2) { // -
    return H_Minus(rand1, rand2);
}

Value H_Mult(const Value &rand1, const Value &rand2) { // *
    //TODO: To complete the Multiplication logic
    if (isNum(rand1) && isNum(rand2)) {
        Rational addend1 = toRational(rand1);
        Rational addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        Rational ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return IntegerV(ans.numerator);
        }
        return RationalV(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Value Mult::evalRator(const Value &rand1, const Value &rand2) { // *
    //TODO: To complete the Multiplication logic
    return H_Mult(rand1, rand2);
}

Value H_Div(const Value &rand1, const Value &rand2) { // /
    //TODO: To complete the dicision logic
    if (isNum(rand1) && isNum(rand2)) {
        Rational addend1 = toRational(rand1);
        Rational addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        if (numerator2 == 0) {
            throw(RuntimeError("Division by zero"));
        }
        int numerator3 = numerator1 * denominator2;
        int denominator3 = denominator1 * numerator2;
        Rational ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return IntegerV(ans.numerator);
        }
        return RationalV(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Value Div::evalRator(const Value &rand1, const Value &rand2) { // /
    return H_Div(rand1, rand2);
}

Value Modulo::evalRator(const Value &rand1, const Value &rand2) { // modulo
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        int dividend = static_cast<Integer*>(rand1.get())->n;
        int divisor = static_cast<Integer*>(rand2.get())->n;
        if (divisor == 0) {
            throw(RuntimeError("Division by zero"));
        }
        return IntegerV(dividend % divisor);
    }
    throw(RuntimeError("modulo is only defined for integers"));
}

Value PlusVar::evalRator(const std::vector<Value> &args) { // + with multiple args
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Plus);
}

Value MinusVar::evalRator(const std::vector<Value> &args) { // - with multiple args
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Minus);
}

Value MultVar::evalRator(const std::vector<Value> &args) { // * with multiple args
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Mult);
}

Value DivVar::evalRator(const std::vector<Value> &args) { // / with multiple args
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Div);
}

Value Expt::evalRator(const Value &rand1, const Value &rand2) { // expt
    if (rand1->v_type == V_INT and rand2->v_type == V_INT) {
        int base = dynamic_cast<Integer*>(rand1.get())->n;
        int exponent = dynamic_cast<Integer*>(rand2.get())->n;
        
        if (exponent < 0) {
            throw(RuntimeError("Negative exponent not supported for integers"));
        }
        if (base == 0 && exponent == 0) {
            throw(RuntimeError("0^0 is undefined"));
        }
        
        long long result = 1;
        long long b = base;
        int exp = exponent;
        
        while (exp > 0) {
            if (exp % 2 == 1) {
                result *= b;
                if (result > INT_MAX || result < INT_MIN) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            b *= b;
            if (b > INT_MAX || b < INT_MIN) {
                if (exp > 1) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            exp /= 2;
        }
        
        return IntegerV((int)result);
    }
    throw(RuntimeError("Wrong typename"));
}

//A FUNCTION TO SIMPLIFY THE COMPARISON WITH INTEGER AND RATIONAL NUMBER
int compareNumericValues(const Value &v1, const Value &v2) {
    if (v1->v_type == V_INT && v2->v_type == V_INT) {
        int n1 = dynamic_cast<Integer*>(v1.get())->n;
        int n2 = dynamic_cast<Integer*>(v2.get())->n;
        return (n1 < n2) ? -1 : (n1 > n2) ? 1 : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_INT) {
        Rational* r1 = dynamic_cast<Rational*>(v1.get());
        int n2 = dynamic_cast<Integer*>(v2.get())->n;
        int left = r1->numerator;
        int right = n2 * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->v_type == V_INT && v2->v_type == V_RATIONAL) {
        int n1 = dynamic_cast<Integer*>(v1.get())->n;
        Rational* r2 = dynamic_cast<Rational*>(v2.get());
        int left = n1 * r2->denominator;
        int right = r2->numerator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_RATIONAL) {
        Rational* r1 = dynamic_cast<Rational*>(v1.get());
        Rational* r2 = dynamic_cast<Rational*>(v2.get());
        int left = r1->numerator * r2->denominator;
        int right = r2->numerator * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    throw RuntimeError("Wrong typename in numeric comparison");
}

bool H_Less(const Value &rand1, const Value &rand2) { // <
    int res = compareNumericValues(rand1, rand2);
    return res == -1;
}

Value Less::evalRator(const Value &rand1, const Value &rand2) { // <
    return BooleanV(H_Less(rand1, rand2));
}

bool H_LessEq(const Value &rand1, const Value &rand2) { // <=
    int res = compareNumericValues(rand1, rand2);
    return res == -1 || res == 0;
}

Value LessEq::evalRator(const Value &rand1, const Value &rand2) { // <=
    return BooleanV(H_LessEq(rand1, rand2));
}

bool H_Equal(const Value &rand1, const Value &rand2) { // =
    int res = compareNumericValues(rand1, rand2);
    return res == 0;
}

Value Equal::evalRator(const Value &rand1, const Value &rand2) { // =
    return BooleanV(H_Equal(rand1, rand2));
}

bool H_GreaterEq(const Value &rand1, const Value &rand2) { // >=
    int res = compareNumericValues(rand1, rand2);
    return res == 0 || res == 1;
}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) { // >=
    return BooleanV(H_GreaterEq(rand1, rand2));
}

bool H_Greater(const Value &rand1, const Value &rand2) { // >
    int res = compareNumericValues(rand1, rand2);
    return res == 1;
}

Value Greater::evalRator(const Value &rand1, const Value &rand2) { // >
    return BooleanV(H_Greater(rand1, rand2));
}

// 输入比较函数，输出多变量比较函数
std::function<Value(const std::vector<Value> &args)> VarFactory(std::function<bool(const Value &rand1, const Value &rand2)> cmp) {
    return [cmp](const std::vector<Value> &args) {
        bool sorted = std::accumulate(
            args.begin() + 1, args.end(), true,
            [cmp, it = args.begin()](bool acc, const Value& x) mutable {
                bool ok = cmp(*it, x);
                ++it;
                return acc && ok;
            }
        );
        return BooleanV(sorted);
    };
}

static auto H_LessVar = VarFactory(H_Less);
static auto H_LessEqVar = VarFactory(H_LessEq);
static auto H_EqualVar = VarFactory(H_Equal);
static auto H_GreaterEqVar = VarFactory(H_GreaterEq);
static auto H_GreaterVar = VarFactory(H_Greater);

Value LessVar::evalRator(const std::vector<Value>& args) { // <= with multiple args
    return H_LessVar(args);
}
Value LessEqVar::evalRator(const std::vector<Value> &args) { // <= with multiple args
    return H_LessEqVar(args);
}
Value EqualVar::evalRator(const std::vector<Value> &args) { // = with multiple args
    return H_EqualVar(args);
}
Value GreaterEqVar::evalRator(const std::vector<Value> &args) { // >= with multiple args
    return H_GreaterEqVar(args);
}
Value GreaterVar::evalRator(const std::vector<Value> &args) { // > with multiple args
    return H_GreaterVar(args);
}

Value Cons::evalRator(const Value &rand1, const Value &rand2) { // cons
    //TODO: To complete the cons logic
}

Value ListFunc::evalRator(const std::vector<Value> &args) { // list function
    //TODO: To complete the list logic
}

Value IsList::evalRator(const Value &rand) { // list?
    //TODO: To complete the list? logic
}

Value Car::evalRator(const Value &rand) { // car
    //TODO: To complete the car logic
}

Value Cdr::evalRator(const Value &rand) { // cdr
    //TODO: To complete the cdr logic
}

Value SetCar::evalRator(const Value &rand1, const Value &rand2) { // set-car!
    //TODO: To complete the set-car! logic
}

Value SetCdr::evalRator(const Value &rand1, const Value &rand2) { // set-cdr!
   //TODO: To complete the set-cdr! logic
}

Value IsEq::evalRator(const Value &rand1, const Value &rand2) { // eq?
    // 检查类型是否为 Integer
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV((dynamic_cast<Integer*>(rand1.get())->n) == (dynamic_cast<Integer*>(rand2.get())->n));
    }
    // 检查类型是否为 Boolean
    else if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL) {
        return BooleanV((dynamic_cast<Boolean*>(rand1.get())->b) == (dynamic_cast<Boolean*>(rand2.get())->b));
    }
    // 检查类型是否为 Symbol
    else if (rand1->v_type == V_SYM && rand2->v_type == V_SYM) {
        return BooleanV((dynamic_cast<Symbol*>(rand1.get())->s) == (dynamic_cast<Symbol*>(rand2.get())->s));
    }
    // 检查类型是否为 Null 或 Void
    else if ((rand1->v_type == V_NULL && rand2->v_type == V_NULL) ||
             (rand1->v_type == V_VOID && rand2->v_type == V_VOID)) {
        return BooleanV(true);
    } else {
        return BooleanV(rand1.get() == rand2.get());
    }
}

Value IsBoolean::evalRator(const Value &rand) { // boolean?
    return BooleanV(rand->v_type == V_BOOL);
}

Value IsFixnum::evalRator(const Value &rand) { // number?
    return BooleanV(rand->v_type == V_INT);
}

Value IsNull::evalRator(const Value &rand) { // null?
    return BooleanV(rand->v_type == V_NULL);
}

Value IsPair::evalRator(const Value &rand) { // pair?
    return BooleanV(rand->v_type == V_PAIR);
}

Value IsProcedure::evalRator(const Value &rand) { // procedure?
    return BooleanV(rand->v_type == V_PROC);
}

Value IsSymbol::evalRator(const Value &rand) { // symbol?
    return BooleanV(rand->v_type == V_SYM);
}

Value IsString::evalRator(const Value &rand) { // string?
    return BooleanV(rand->v_type == V_STRING);
}

Value Begin::eval(Assoc &e) {
    //TODO: To complete the begin logic
}

Value Quote::eval(Assoc& e) {
    //TODO: To complete the quote logic
}

Value AndVar::eval(Assoc &e) { // and with short-circuit evaluation
    //TODO: To complete the and logic
}

Value OrVar::eval(Assoc &e) { // or with short-circuit evaluation
    //TODO: To complete the or logic
}

Value Not::evalRator(const Value &rand) { // not
    //TODO: To complete the not logic
    
    if (rand->v_type != V_BOOL) {
        throw(RuntimeError("Wrong typename"));
    }
    bool in = static_cast<Boolean*>(rand.get())->b;
    return BooleanV(!in);
}

Value If::eval(Assoc &e) {
    //TODO: To complete the if logic
}

Value Cond::eval(Assoc &env) {
    //TODO: To complete the cond logic
}

Value Lambda::eval(Assoc &env) { 
    //TODO: To complete the lambda logic
}

Value Apply::eval(Assoc &e) {
    /*
    if (rator->eval(e)->v_type != V_PROC) {throw RuntimeError("Attempt to apply a non-procedure");}

    //TODO: TO COMPLETE THE CLOSURE LOGIC
    Procedure* clos_ptr = ;
    
    //TODO: TO COMPLETE THE ARGUMENT PARSER LOGIC
    std::vector<Value> args;
    if (args.size() != clos_ptr->parameters.size()) {throw RuntimeError("Wrong number of arguments");}

    //TODO: TO COMPLETE THE PARAMETERS' ENVIRONMENT LOGIC
    Assoc param_env = ;

    return clos_ptr->e->eval(param_env);
    */
}

Value Define::eval(Assoc &env) {
    //TODO: To complete the define logic
}

Value Let::eval(Assoc &env) {
    //TODO: To complete the let logic
}

Value Letrec::eval(Assoc &env) {
    //TODO: To complete the letrec logic
}

Value Set::eval(Assoc &env) {
    //TODO: To complete the set logic
}

Value Display::evalRator(const Value &rand) { // display function
    if (rand->v_type == V_STRING) {
        String* str_ptr = dynamic_cast<String*>(rand.get());
        std::cout << str_ptr->s;
    } else {
        rand->show(std::cout);
    }
    
    return VoidV();
}
