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

Value NullExpr::eval(Assoc &e) {
    return NullV();
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
            return PrimitiveV(primitives[x]);
        }
        else if (reserved_words.count(x)) {
            return SpecialFormV(reserved_words[x]);
        }
        else {
            throw(RuntimeError("undefined variable"));
        }
    }
    return matched_value;
}

Value SList::eval(Assoc &e) {
    Value p = terms[0]->eval(e);
    if (p->v_type == V_PROC) {
        Procedure* clos_ptr = static_cast<Procedure*>(p.get());
        std::vector<Value> args;
        std::transform(
            terms.begin() + 1, terms.end(), std::back_inserter(args),
            [&e](Expr x){
                return x->eval(e);
            }
        );
        return (Expr(new Apply(p, args)))->eval(e);
    } else if (p->v_type == V_PRIMITIVE) {
        auto op = static_cast<Primitive*>(p.get());
        auto rand = std::vector<Expr>(terms.begin() + 1, terms.end());
        switch (op->type)
        {
            case E_PLUS:
            return Expr(new PlusVar(rand))->eval(e);
            case E_MINUS:
            return rand.size() >= 1 ? Expr(new MinusVar(rand))->eval(e): throw RuntimeError("Wrong number of arguments for -");
            case E_MUL:
            return Expr(new MultVar(rand))->eval(e);
            case E_DIV:
            return rand.size() >= 1 ? Expr(new DivVar(rand))->eval(e): throw RuntimeError("Wrong number of arguments for /");
            case E_MODULO:
            return rand.size() == 2 ? Expr(new Modulo(rand[0], rand[1]))->eval(e): throw RuntimeError("Wrong number of arguments for modulo");
            case E_EXPT:
            return rand.size() == 2 ? Expr(new Expt(rand[0], rand[1]))->eval(e): throw RuntimeError("Wrong number of arguments for expt");
            // Comparison operations
            case E_LT:
            return Expr(new LessVar(rand))->eval(e);
            case E_LE:
            return Expr(new LessEqVar(rand))->eval(e);
            case E_EQ: 
            return Expr(new EqualVar(rand))->eval(e);
            case E_GE: 
            return Expr(new GreaterEqVar(rand))->eval(e);
            case E_GT: 
            return Expr(new GreaterVar(rand))->eval(e);
            // Logic operations
            case E_NOT:
            return rand.size() == 1 ? Expr(new Not(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for not");
            case E_AND:
            return Expr(new AndVar(rand))->eval(e);
            case E_OR:
            return Expr(new OrVar(rand))->eval(e);
            // List operations
            case E_CONS:
            return rand.size() == 2 ? Expr(new Cons(rand[0], rand[1]))->eval(e): throw RuntimeError("Wrong number of arguments for cons");
            case E_CAR:
            return rand.size() == 1 ? Expr(new Car(rand[0]))->eval(e) : throw RuntimeError("Wrong number of arguments for car");
            case E_CDR:
            return rand.size() == 1 ? Expr(new Cdr(rand[0]))->eval(e) : throw RuntimeError("Wrong number of arguments for cdr");
            case E_LIST:
            return Expr(new ListFunc(rand))->eval(e);
            // Type predicates
            case E_EQQ:
            return rand.size() == 2 ? Expr(new IsEq(rand[0], rand[1]))->eval(e): throw RuntimeError("Wrong number of arguments for eq?");
            case E_BOOLQ:
            return rand.size() == 1 ? Expr(new IsBoolean(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for boolean?");
            case E_INTQ:
            return rand.size() == 1 ? Expr(new IsFixnum(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for number??");
            case E_NULLQ:
            return rand.size() == 1 ? Expr(new IsNull(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for null?");
            case E_PAIRQ:
            return rand.size() == 1 ? Expr(new IsPair(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for pair?");
            case E_PROCQ:
            return rand.size() == 1 ? Expr(new IsProcedure(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for procedure?");
            case E_SYMBOLQ:
            return rand.size() == 1 ? Expr(new IsSymbol(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for symbol?");
            case E_LISTQ: 
            return rand.size() == 1 ? Expr(new IsList(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for list?");
            case E_STRINGQ:
            return rand.size() == 1 ? Expr(new IsString(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for string?");
            // Special values and control
            case E_VOID:
            return rand.size() == 0 ? Expr(new MakeVoid())->eval(e): throw RuntimeError("Wrong number of arguments for void");
            case E_EXIT:
            return rand.size() == 0 ? Expr(new Exit())->eval(e): throw RuntimeError("Wrong number of arguments for exit");
            default:
                break;
        }
    }
    else if (p->v_type == V_SPECIALFORM) {
        auto op = static_cast<SpecialForm*>(p.get());
        auto rand = std::vector<Expr>(terms.begin() + 1, terms.end());
        switch (op->type) { 
            // Control flow constructs
            case E_BEGIN:
            return Expr(new Begin(rand))->eval(e);
            case E_QUOTE:
            return rand.size() == 1 ? Expr(new Quote(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for quote");
            //Conditional
            case E_IF:
            return rand.size() == 3 ? Expr(new If(rand[0], rand[1], rand[2]))->eval(e) : throw RuntimeError("Wrong number of arguments for if");
            case E_COND:
            break;
            // Variables and function definition
            case E_LAMBDA:
            {
                if (rand.size() != 2) throw(RuntimeError("Wrong number of arguments for lambda"));
                std::vector<std::string> paras;
                auto symbolsList = dynamic_cast<SList*>(rand[0].get());
                if (symbolsList == nullptr) throw(RuntimeError("lambda takes a list as the 1st parameter"));
                auto symbols = symbolsList->terms;
                std::transform(symbols.begin(), symbols.end(), std::back_inserter(paras),[](Expr x) {
                    auto y = dynamic_cast<Var*>(x.get()); 
                    if (y == nullptr) throw(RuntimeError("lambda parameter is not symbol"));
                    return y->x;
                });
                return Expr(new Lambda(paras, rand[1]))->eval(e);
            }
            case E_DEFINE:
            {
                if (rand.size() != 2) throw(RuntimeError("Wrong number of arguments for define"));
                auto symbol = dynamic_cast<Var*>(rand[0].get());
                // call Define
                if (symbol != nullptr) {
                    std::string variable = symbol->x;
                    if (primitives.count(variable) || reserved_words.count(variable)) throw(RuntimeError("variable names can't be primitives or reserve_words"));
                    return Expr(new Define(variable, rand[1]))->eval(e);
                }
                // call Define_f
                auto symbolsList = dynamic_cast<SList*>(rand[0].get());
                if (symbolsList == nullptr) throw(RuntimeError("define takes a symbol or list as the 1st parameter"));
                auto symbols = symbolsList->terms;

                auto function_name = dynamic_cast<Var*>(symbols[0].get());
                if (function_name == nullptr) throw(RuntimeError("lambda name is not symbol"));
                std::string variable = function_name->x;
                //if (primitives.count(variable) || reserved_words.count(variable)) throw(RuntimeError("variable names can't be primitives or reserve_words"));

                std::vector<std::string> paras;
                std::transform(symbols.begin() + 1, symbols.end(), std::back_inserter(paras),[](Expr x) {
                    auto y = dynamic_cast<Var*>(x.get()); 
                    if (y == nullptr) throw(RuntimeError("lambda parameter is not symbol"));
                    return y->x;
                });
                Expr body = rand[1];
                return Expr(new Define_f(variable, paras, body))->eval(e);
            }
            // Binding constructs
            case E_LET:
            {
                if (rand.size() != 2) throw(RuntimeError("Wrong number of arguments for let"));
                auto pairList = dynamic_cast<SList*>(rand[0].get());
                if (pairList == nullptr) throw(RuntimeError("let takes a list as the 1st parameter"));
                auto pairs = pairList->terms;

                std::vector<std::pair<std::string, Expr>> result;
                std::transform(pairs.begin(), pairs.end(), std::back_inserter(result),[](Expr x) {
                    auto y = dynamic_cast<SList*>(x.get()); 
                    if (y != nullptr) {
                        auto s = y->terms;
                        if (s.size() == 2) {
                            auto s1 = dynamic_cast<Var*>(s[0].get());
                            auto s2 = s[1];
                            if (s1 != nullptr) {
                                return std::make_pair(s1->x, s2);
                            }
                        }
                    }
                    throw(RuntimeError("Wrong form of arguments for let"));
                });
                return Expr(new Let(result, rand[1]))->eval(e);
            }
            case E_LETREC:
            // Assignment
            case E_SET:
        	default:
            	throw RuntimeError("Unknown reserved word: " + op->type);
    	}
    }
    throw RuntimeError("Attempt to apply a non-procedure");    
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
    return std::accumulate(args.begin(), args.end(), IntegerV(0), H_Plus);
}

Value MinusVar::evalRator(const std::vector<Value> &args) { // - with multiple args
    if (args.size() == 1) return H_Minus(IntegerV(0), args[0]);
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Minus);
}

Value MultVar::evalRator(const std::vector<Value> &args) { // * with multiple args
    return std::accumulate(args.begin(), args.end(), IntegerV(1), H_Mult);
}

Value DivVar::evalRator(const std::vector<Value> &args) { // / with multiple args
    if (args.size() == 1) return H_Div(IntegerV(1), args[0]);
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
    return PairV(rand1, rand2);
}

Value ListFunc::evalRator(const std::vector<Value> &args) { // list function
    //TODO: To complete the list logic
    return std::accumulate(
        args.rbegin(), args.rend(), NullV(),
        [](Value tail, const Value& x) {
            return PairV(x, tail);
        }
    );
}

bool H_IsList(const Value &rand) {
    return rand->v_type == V_NULL || rand->v_type == V_PAIR && H_IsList(static_cast<Pair*>(rand.get())->cdr);
}

Value IsList::evalRator(const Value &rand) { // list?
    //TODO: To complete the list? logic
    return BooleanV(H_IsList(rand));
}

Value Car::evalRator(const Value &rand) { // car
    //TODO: To complete the car logic
    if (rand->v_type != V_PAIR) {
        throw(RuntimeError("Wrong typename"));
    }
    return static_cast<Pair*>(rand.get())->car;
}

Value Cdr::evalRator(const Value &rand) { // cdr
    //TODO: To complete the cdr logic
        if (rand->v_type != V_PAIR) {
        throw(RuntimeError("Wrong typename"));
    }
    return static_cast<Pair*>(rand.get())->cdr;
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

    auto p = es.begin(), q = es.end() - 1;
    while (p != q) {
        (*p)->eval(e);
        p++;
    }
    return (*q)->eval(e);
}

Value Quote::eval(Assoc& e) {
    //TODO: To complete the quote logic
    return ex->eval(e);
}

Value AndVar::eval(Assoc &e) { // and with short-circuit evaluation
    // Scheme semantics:
    // - (and) => #t
    // - Evaluate left-to-right; on first #f, return #f without evaluating rest
    // - If all are truthy, return the last evaluated value
    if (rands.empty()) return BooleanV(true);

    Value last = BooleanV(true);
    for (const auto &ex : rands) {
        last = ex->eval(e);
        if (last->v_type == V_BOOL && !static_cast<Boolean*>(last.get())->b) {
            return BooleanV(false);
        }
    }
    return last;
}

Value OrVar::eval(Assoc &e) { // or with short-circuit evaluation
    //TODO: To complete the or logic
    if (rands.empty()) return BooleanV(false);

    Value last = BooleanV(false);
    for (const auto &ex : rands) {
        last = ex->eval(e);
        if (last->v_type == V_BOOL && static_cast<Boolean*>(last.get())->b) {
            return BooleanV(true);
        }
    }
    return last;
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
    Value cond_res = cond->eval(e);

    if (cond_res->v_type == V_BOOL && !static_cast<Boolean*>(cond_res.get())->b) {
        return alter->eval(e);
    }
    return conseq->eval(e);
}

Value Cond::eval(Assoc &env) {
    //TODO: To complete the cond logic

}

Value Lambda::eval(Assoc &env) { 
    //TODO: To complete the lambda logic
    return ProcedureV(x, e, env);
}

Value Apply::eval(Assoc &e) {
    auto p = static_cast<Procedure*>(rator.get());
    if (rand.size() != p->parameters.size()) {throw RuntimeError("Wrong number of arguments");}

    Assoc param_env = p->env;
    for (int i = 0; i < rand.size(); i++) {
        param_env = extend(p->parameters[i], rand[i], param_env);
    }
    return p->e->eval(param_env);
}

Value Define::eval(Assoc &env) {
    //TODO: To complete the define logic
    env = extend(var, e->eval(env), env);
    return Value(nullptr);
}

Value Define_f::eval(Assoc &env) {
    env = extend(var, VoidV(), env);
    env->v = ProcedureV(x, e, env);
    return Value(nullptr);
}

Value Let::eval(Assoc &env) {
    //TODO: To complete the let logic
    Assoc param_env = env;
    
    for (auto b : bind) {
        param_env = extend(b.first, b.second->eval(env), param_env);
    }

    return body->eval(param_env);
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

Value Quoted_Symbol::eval(Assoc &env) {
    return SymbolV(var);
}