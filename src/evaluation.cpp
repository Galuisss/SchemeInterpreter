/**
 * @file evaluation.cpp
 * @brief Expression evaluation implementation for the Scheme interpreter
 * @author luke36
 * 
 * This file implements evaluation methods for all expression types in the Scheme
 * interpreter. Functions are organized according to ExprType enumeration order
 * from Def.hpp for consistency and maintainability.
 */

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


Expr self_evaluating::eval(Assoc &) {
    return Expr(this);
}

Expr Unary::eval(Assoc &e) { // evaluation of single-operator primitive
    return evalRator(rand->eval(e));
}

Expr Binary::eval(Assoc &e) { // evaluation of two-operators primitive
    return evalRator(rand1->eval(e), rand2->eval(e));
}

Expr Variadic::eval(Assoc &e) { // evaluation of multi-operator primitive
    // TODO: TO COMPLETE THE VARIADIC CLASS
    std::vector<Expr> results;
    results.reserve(rands.size());
    std::transform(rands.begin(), rands.end(), std::back_inserter(results), [&e](const Expr &x){return x->eval(e);});
    return evalRator(results);
}

Expr Var::eval(Assoc &e) { // evaluation of variable
    // TODO: TO identify the invalid variable
    // We request all valid variable just need to be a Var,you should promise:
    //The first character of a variable name cannot be a digit or any character from the set: {.@}
    //If a string can be recognized as a number, it will be prioritized as a number. For example: 1, -1, +123, .123, +124., 1e-3
    //Variable names can overlap with primitives and reserve_words
    //Variable names can contain any non-whitespace characters except #, ', ", `, but the first character cannot be a digit
    //When a variable is not defined in the current scope, your interpreter should output RuntimeError
    
    Expr matched_Expr = find(x, e);
    if (matched_Expr.get() == nullptr) {
        if (primitives.count(x)) {
            return PrimitiveE(primitives[x]);
        }
        else if (reserved_words.count(x)) {
            return SpecialFormE(reserved_words[x]);
        }
        else {
            throw(RuntimeError("undefined variable"));
        }
    }
    return matched_Expr;
}

Expr Quoted(const Expr&e) {
    auto list = dynamic_cast<SList*>(e.get());
    if (list == nullptr) return e;

    auto terms = list->terms;
    if (terms.size() >= 3) {
        auto s = *(terms.rbegin() + 1);
        auto ss = dynamic_cast<Var*>(s.get());
        if (ss != nullptr && ss->x == ".") {
            Expr ex = Quoted(*(terms.rbegin()));
            return std::accumulate(
            (terms.rbegin() + 2), terms.rend(), ex,
            [](Expr tail, const Expr& x) {
                return PairE(Quoted(x), tail);
            });
        }
    }

    Expr ex = NullExprE();
    return std::accumulate(
        terms.rbegin(), terms.rend(), ex,
        [](Expr tail, const Expr& x) {
            return PairE(Quoted(x), tail);
        });
}

Expr SList::eval(Assoc &e) {
    Expr p = terms[0]->eval(e);
    if (p->e_type == E_PROC) {
        Procedure* clos_ptr = static_cast<Procedure*>(p.get());
        std::vector<Expr> args;
        std::transform(
            terms.begin() + 1, terms.end(), std::back_inserter(args),
            [&e](Expr x){
                return x->eval(e);
            }
        );
        return (Expr(new Apply(p, args)))->eval(e);
    } else if (p->e_type == E_PRIMITIVE) {
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
            return rand.size() == 1 ? Expr(new IsSymbol(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for Var?");
            case E_LISTQ: 
            return rand.size() == 1 ? Expr(new IsList(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for list?");
            case E_STRINGQ:
            return rand.size() == 1 ? Expr(new IsString(rand[0]))->eval(e): throw RuntimeError("Wrong number of arguments for string?");
            // Special Exprs and control
            case E_VOID:
            return rand.size() == 0 ? Expr(new MakeVoid())->eval(e): throw RuntimeError("Wrong number of arguments for void");
            case E_EXIT:
            return rand.size() == 0 ? Expr(new Exit())->eval(e): throw RuntimeError("Wrong number of arguments for exit");
            default:
                break;
        }
    }
    else if (p->e_type == E_SPECIALFORM) {
        auto op = static_cast<SpecialForm*>(p.get());
        auto rand = std::vector<Expr>(terms.begin() + 1, terms.end());
        switch (op->type) { 
            // Control flow constructs
            case E_BEGIN:
            return Expr(new Begin(rand))->eval(e);
            case E_QUOTE:
            {
            return rand.size() == 1 ? Quoted(rand[0]): throw RuntimeError("Wrong number of arguments for quote");
            }
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
                auto VarsList = dynamic_cast<SList*>(rand[0].get());
                if (VarsList == nullptr) throw(RuntimeError("lambda takes a list as the 1st parameter"));
                auto Vars = VarsList->terms;
                std::transform(Vars.begin(), Vars.end(), std::back_inserter(paras),[](Expr x) {
                    auto y = dynamic_cast<Var*>(x.get()); 
                    if (y == nullptr) throw(RuntimeError("lambda parameter is not Var"));
                    return y->x;
                });
                return Expr(new Lambda(paras, rand[1]))->eval(e);
            }
            case E_DEFINE:
            {
                if (rand.size() != 2) throw(RuntimeError("Wrong number of arguments for define"));
                auto var = dynamic_cast<Var*>(rand[0].get());
                // call Define
                if (var != nullptr) {
                    std::string variable = var->x;
                    if (primitives.count(variable) || reserved_words.count(variable)) throw(RuntimeError("variable names can't be primitives or reserve_words"));
                    return Expr(new Define(variable, rand[1]))->eval(e);
                }
                // call Define_f
                auto VarsList = dynamic_cast<SList*>(rand[0].get());
                if (VarsList == nullptr) throw(RuntimeError("define takes a Var or list as the 1st parameter"));
                auto Vars = VarsList->terms;

                auto function_name = dynamic_cast<Var*>(Vars[0].get());
                if (function_name == nullptr) throw(RuntimeError("lambda name is not Var"));
                std::string variable = function_name->x;
                //if (primitives.count(variable) || reserved_words.count(variable)) throw(RuntimeError("variable names can't be primitives or reserve_words"));

                std::vector<std::string> paras;
                std::transform(Vars.begin() + 1, Vars.end(), std::back_inserter(paras),[](Expr x) {
                    auto y = dynamic_cast<Var*>(x.get()); 
                    if (y == nullptr) throw(RuntimeError("lambda parameter is not Var"));
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

bool isInt(const Expr &v) {
    return v->e_type == E_FIXNUM;
}

bool isRat(const Expr &v) {
    return v->e_type == E_RATIONAL;
}

bool isNum(const Expr &v) {
    return isInt(v) || isRat(v);
}
RationalNum toRational(const Expr& v) {
    switch (v->e_type) {
    case E_FIXNUM:
        return RationalNum(*static_cast<Fixnum*>(v.get()));
    case E_RATIONAL:
        return *static_cast<RationalNum*>(v.get());
    default:
        throw std::runtime_error("Not a number");
    }
}

Expr H_Plus(const Expr &rand1, const Expr &rand2) {
    if (isNum(rand1) && isNum(rand2)) {
        RationalNum addend1 = toRational(rand1);
        RationalNum addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * denominator2 + denominator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        RationalNum ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return FixnumE(ans.numerator);
        }
        return RationalNumE(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Expr Plus::evalRator(const Expr &rand1, const Expr &rand2) { // +
    //TODO: To complete the addition logic
    return H_Plus(rand1, rand2);
}

Expr H_Minus(const Expr &rand1, const Expr &rand2) { // -
    //TODO: To complete the substraction logic
    if (isNum(rand1) && isNum(rand2)) {
        RationalNum addend1 = toRational(rand1);
        RationalNum addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * denominator2 - denominator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        RationalNum ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return FixnumE(ans.numerator);
        }
        return RationalNumE(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Expr Minus::evalRator(const Expr &rand1, const Expr &rand2) { // -
    return H_Minus(rand1, rand2);
}

Expr H_Mult(const Expr &rand1, const Expr &rand2) { // *
    //TODO: To complete the Multiplication logic
    if (isNum(rand1) && isNum(rand2)) {
        RationalNum addend1 = toRational(rand1);
        RationalNum addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        int numerator3 = numerator1 * numerator2;
        int denominator3 = denominator1 * denominator2;
        RationalNum ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return FixnumE(ans.numerator);
        }
        return RationalNumE(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Expr Mult::evalRator(const Expr &rand1, const Expr &rand2) { // *
    //TODO: To complete the Multiplication logic
    return H_Mult(rand1, rand2);
}

Expr H_Div(const Expr &rand1, const Expr &rand2) { // /
    //TODO: To complete the dicision logic
    if (isNum(rand1) && isNum(rand2)) {
        RationalNum addend1 = toRational(rand1);
        RationalNum addend2 = toRational(rand2);
        int numerator1 = addend1.numerator, denominator1 = addend1.denominator;
        int numerator2 = addend2.numerator, denominator2 = addend2.denominator;
        if (numerator2 == 0) {
            throw(RuntimeError("Division by zero"));
        }
        int numerator3 = numerator1 * denominator2;
        int denominator3 = denominator1 * numerator2;
        RationalNum ans(numerator3, denominator3);

        if (ans.denominator == 1) {
            return FixnumE(ans.numerator);
        }
        return RationalNumE(numerator3, denominator3);
    }
    throw(RuntimeError("Wrong typename"));
}

Expr Div::evalRator(const Expr &rand1, const Expr &rand2) { // /
    return H_Div(rand1, rand2);
}

Expr Modulo::evalRator(const Expr &rand1, const Expr &rand2) { // modulo
    if (rand1->e_type == E_FIXNUM && rand2->e_type == E_FIXNUM) {
        int dividend = static_cast<Fixnum*>(rand1.get())->n;
        int divisor = static_cast<Fixnum*>(rand2.get())->n;
        if (divisor == 0) {
            throw(RuntimeError("Division by zero"));
        }
        return FixnumE(dividend % divisor);
    }
    throw(RuntimeError("modulo is only defined for Fixnums"));
}

Expr PlusVar::evalRator(const std::vector<Expr> &args) { // + with multiple args
    return std::accumulate(args.begin(), args.end(), FixnumE(0), H_Plus);
}

Expr MinusVar::evalRator(const std::vector<Expr> &args) { // - with multiple args
    if (args.size() == 1) return H_Minus(FixnumE(0), args[0]);
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Minus);
}

Expr MultVar::evalRator(const std::vector<Expr> &args) { // * with multiple args
    return std::accumulate(args.begin(), args.end(), FixnumE(1), H_Mult);
}

Expr DivVar::evalRator(const std::vector<Expr> &args) { // / with multiple args
    if (args.size() == 1) return H_Div(FixnumE(1), args[0]);
    return std::accumulate(args.begin() + 1, args.end(), args[0], H_Div);
}

Expr Expt::evalRator(const Expr &rand1, const Expr &rand2) { // expt
    if (rand1->e_type == E_FIXNUM and rand2->e_type == E_FIXNUM) {
        int base = dynamic_cast<Fixnum*>(rand1.get())->n;
        int exponent = dynamic_cast<Fixnum*>(rand2.get())->n;
        
        if (exponent < 0) {
            throw(RuntimeError("Negative exponent not supported for Fixnums"));
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
                    throw(RuntimeError("Fixnum overflow in expt"));
                }
            }
            b *= b;
            if (b > INT_MAX || b < INT_MIN) {
                if (exp > 1) {
                    throw(RuntimeError("Fixnum overflow in expt"));
                }
            }
            exp /= 2;
        }
        
        return FixnumE((int)result);
    }
    throw(RuntimeError("Wrong typename"));
}

//A FUNCTION TO SIMPLIFY THE COMPARISON WITH Fixnum AND RATIONAL NUMBER
int compareNumericExprs(const Expr &v1, const Expr &v2) {
    if (v1->e_type == E_FIXNUM && v2->e_type == E_FIXNUM) {
        int n1 = dynamic_cast<Fixnum*>(v1.get())->n;
        int n2 = dynamic_cast<Fixnum*>(v2.get())->n;
        return (n1 < n2) ? -1 : (n1 > n2) ? 1 : 0;
    }
    else if (v1->e_type == E_RATIONAL && v2->e_type == E_FIXNUM) {
        RationalNum* r1 = dynamic_cast<RationalNum*>(v1.get());
        int n2 = dynamic_cast<Fixnum*>(v2.get())->n;
        int left = r1->numerator;
        int right = n2 * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->e_type == E_FIXNUM && v2->e_type == E_RATIONAL) {
        int n1 = dynamic_cast<Fixnum*>(v1.get())->n;
        RationalNum* r2 = dynamic_cast<RationalNum*>(v2.get());
        int left = n1 * r2->denominator;
        int right = r2->numerator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    else if (v1->e_type == E_RATIONAL && v2->e_type == E_RATIONAL) {
        RationalNum* r1 = dynamic_cast<RationalNum*>(v1.get());
        RationalNum* r2 = dynamic_cast<RationalNum*>(v2.get());
        int left = r1->numerator * r2->denominator;
        int right = r2->numerator * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1 : 0;
    }
    throw RuntimeError("Wrong typename in numeric comparison");
}

bool H_Less(const Expr &rand1, const Expr &rand2) { // <
    int res = compareNumericExprs(rand1, rand2);
    return res == -1;
}

Expr Less::evalRator(const Expr &rand1, const Expr &rand2) { // <
    return BooleanE(H_Less(rand1, rand2));
}

bool H_LessEq(const Expr &rand1, const Expr &rand2) { // <=
    int res = compareNumericExprs(rand1, rand2);
    return res == -1 || res == 0;
}

Expr LessEq::evalRator(const Expr &rand1, const Expr &rand2) { // <=
    return BooleanE(H_LessEq(rand1, rand2));
}

bool H_Equal(const Expr &rand1, const Expr &rand2) { // =
    int res = compareNumericExprs(rand1, rand2);
    return res == 0;
}

Expr Equal::evalRator(const Expr &rand1, const Expr &rand2) { // =
    return BooleanE(H_Equal(rand1, rand2));
}

bool H_GreaterEq(const Expr &rand1, const Expr &rand2) { // >=
    int res = compareNumericExprs(rand1, rand2);
    return res == 0 || res == 1;
}

Expr GreaterEq::evalRator(const Expr &rand1, const Expr &rand2) { // >=
    return BooleanE(H_GreaterEq(rand1, rand2));
}

bool H_Greater(const Expr &rand1, const Expr &rand2) { // >
    int res = compareNumericExprs(rand1, rand2);
    return res == 1;
}

Expr Greater::evalRator(const Expr &rand1, const Expr &rand2) { // >
    return BooleanE(H_Greater(rand1, rand2));
}

// 输入比较函数，输出多变量比较函数
std::function<Expr(const std::vector<Expr> &args)> VarFactory(std::function<bool(const Expr &rand1, const Expr &rand2)> cmp) {
    return [cmp](const std::vector<Expr> &args) {
        bool sorted = std::accumulate(
            args.begin() + 1, args.end(), true,
            [cmp, it = args.begin()](bool acc, const Expr& x) mutable {
                bool ok = cmp(*it, x);
                ++it;
                return acc && ok;
            }
        );
        return BooleanE(sorted);
    };
}

static auto H_LessVar = VarFactory(H_Less);
static auto H_LessEqVar = VarFactory(H_LessEq);
static auto H_EqualVar = VarFactory(H_Equal);
static auto H_GreaterEqVar = VarFactory(H_GreaterEq);
static auto H_GreaterVar = VarFactory(H_Greater);

Expr LessVar::evalRator(const std::vector<Expr>& args) { // <= with multiple args
    return H_LessVar(args);
}
Expr LessEqVar::evalRator(const std::vector<Expr> &args) { // <= with multiple args
    return H_LessEqVar(args);
}
Expr EqualVar::evalRator(const std::vector<Expr> &args) { // = with multiple args
    return H_EqualVar(args);
}
Expr GreaterEqVar::evalRator(const std::vector<Expr> &args) { // >= with multiple args
    return H_GreaterEqVar(args);
}
Expr GreaterVar::evalRator(const std::vector<Expr> &args) { // > with multiple args
    return H_GreaterVar(args);
}

Expr Cons::evalRator(const Expr &rand1, const Expr &rand2) { // cons
    //TODO: To complete the cons logic
    return PairE(rand1, rand2);
}

Expr ListFunc::evalRator(const std::vector<Expr> &args) { // list function
    //TODO: To complete the list logic
    return std::accumulate(
        args.rbegin(), args.rend(), NullExprE(),
        [](Expr tail, const Expr& x) {
            return PairE(x, tail);
        }
    );
}

bool H_IsList(const Expr &rand) {
    return rand->e_type == E_NULL || rand->e_type == E_PAIR && H_IsList(static_cast<Pair*>(rand.get())->cdr);
}

Expr IsList::evalRator(const Expr &rand) { // list?
    //TODO: To complete the list? logic
    return BooleanE(H_IsList(rand));
}

Expr Car::evalRator(const Expr &rand) { // car
    //TODO: To complete the car logic
    if (rand->e_type != E_PAIR) {
        throw(RuntimeError("Wrong typename"));
    }
    return static_cast<Pair*>(rand.get())->car;
}

Expr Cdr::evalRator(const Expr &rand) { // cdr
    //TODO: To complete the cdr logic
        if (rand->e_type != E_PAIR) {
        throw(RuntimeError("Wrong typename"));
    }
    return static_cast<Pair*>(rand.get())->cdr;
}

Expr SetCar::evalRator(const Expr &rand1, const Expr &rand2) { // set-car!
    //TODO: To complete the set-car! logic
}

Expr SetCdr::evalRator(const Expr &rand1, const Expr &rand2) { // set-cdr!
   //TODO: To complete the set-cdr! logic
}

Expr IsEq::evalRator(const Expr &rand1, const Expr &rand2) { // eq?
    // 检查类型是否为 Fixnum
    if (rand1->e_type == E_FIXNUM && rand2->e_type == E_FIXNUM) {
        return BooleanE((dynamic_cast<Fixnum*>(rand1.get())->n) == (dynamic_cast<Fixnum*>(rand2.get())->n));
    }
    // 检查类型是否为 Boolean
    else if (rand1->e_type == E_BOOLEAN && rand2->e_type == E_BOOLEAN) {
        return BooleanE((dynamic_cast<Boolean*>(rand1.get())->b) == (dynamic_cast<Boolean*>(rand2.get())->b));
    }
    // 检查类型是否为 Var
    else if (rand1->e_type == E_VAR && rand2->e_type == E_VAR) {
        return BooleanE((dynamic_cast<Var*>(rand1.get())->x) == (dynamic_cast<Var*>(rand2.get())->x));
    }
    // 检查类型是否为 Null 或 Void
    else if ((rand1->e_type == E_NULL && rand2->e_type == E_NULL) ||
             (rand1->e_type == E_VOID && rand2->e_type == E_VOID)) {
        return BooleanE(true);
    } else {
        return BooleanE(rand1.get() == rand2.get());
    }
}

Expr IsBoolean::evalRator(const Expr &rand) { // boolean?
    return BooleanE(rand->e_type == E_BOOLEAN);
}

Expr IsFixnum::evalRator(const Expr &rand) { // number?
    return BooleanE(rand->e_type == E_FIXNUM);
}

Expr IsNull::evalRator(const Expr &rand) { // null?
    return BooleanE(rand->e_type == E_NULL);
}

Expr IsPair::evalRator(const Expr &rand) { // pair?
    return BooleanE(rand->e_type == E_PAIR);
}

Expr IsProcedure::evalRator(const Expr &rand) { // procedure?
    return BooleanE(rand->e_type == E_PROC);
}

Expr IsSymbol::evalRator(const Expr &rand) { // Var?
    return BooleanE(rand->e_type == E_VAR);
}

Expr IsString::evalRator(const Expr &rand) { // string?
    return BooleanE(rand->e_type == E_STRING);
}

Expr Begin::eval(Assoc &e) {
    //TODO: To complete the begin logic

    auto p = es.begin(), q = es.end() - 1;
    while (p != q) {
        (*p)->eval(e);
        p++;
    }
    return (*q)->eval(e);
}

Expr Quote::eval(Assoc& e) {
    //TODO: To complete the quote logic
    return ex->eval(e);
}

Expr AndVar::eval(Assoc &e) { // and with short-circuit evaluation
    // Scheme semantics:
    // - (and) => #t
    // - Evaluate left-to-right; on first #f, return #f without evaluating rest
    // - If all are truthy, return the last evaluated Expr
    if (rands.empty()) return BooleanE(true);

    Expr last = BooleanE(true);
    for (const auto &ex : rands) {
        last = ex->eval(e);
        if (last->e_type == E_BOOLEAN && !static_cast<Boolean*>(last.get())->b) {
            return BooleanE(false);
        }
    }
    return last;
}

Expr OrVar::eval(Assoc &e) { // or with short-circuit evaluation
    //TODO: To complete the or logic
    if (rands.empty()) return BooleanE(false);

    Expr last = BooleanE(false);
    for (const auto &ex : rands) {
        last = ex->eval(e);
        if (last->e_type == E_BOOLEAN && static_cast<Boolean*>(last.get())->b) {
            return BooleanE(true);
        }
    }
    return last;
}

Expr Not::evalRator(const Expr &rand) { // not
    //TODO: To complete the not logic
    
    if (rand->e_type != E_BOOLEAN) {
        throw(RuntimeError("Wrong typename"));
    }
    bool in = static_cast<Boolean*>(rand.get())->b;
    return BooleanE(!in);
}

Expr If::eval(Assoc &e) {
    //TODO: To complete the if logic
    Expr cond_res = cond->eval(e);

    if (cond_res->e_type == E_BOOLEAN && !static_cast<Boolean*>(cond_res.get())->b) {
        return alter->eval(e);
    }
    return conseq->eval(e);
}

Expr Cond::eval(Assoc &env) {
    //TODO: To complete the cond logic

}

Expr Lambda::eval(Assoc &env) { 
    //TODO: To complete the lambda logic
    return ProcedureE(x, e, env);
}

Expr Apply::eval(Assoc &e) {
    auto p = static_cast<Procedure*>(rator.get());
    if (rand.size() != p->parameters.size()) {throw RuntimeError("Wrong number of arguments");}

    Assoc param_env = p->env;
    for (int i = 0; i < rand.size(); i++) {
        param_env = extend(p->parameters[i], rand[i], param_env);
    }
    return p->e->eval(param_env);
}

Expr Define::eval(Assoc &env) {
    //TODO: To complete the define logic
    env = extend(var, e->eval(env), env);
    return Expr(nullptr);
}

Expr Define_f::eval(Assoc &env) {
    env = extend(var, MakeVoidE(), env);
    env->v = ProcedureE(x, e, env);
    return Expr(nullptr);
}

Expr Let::eval(Assoc &env) {
    //TODO: To complete the let logic
    Assoc param_env = env;
    
    for (auto b : bind) {
        param_env = extend(b.first, b.second->eval(env), param_env);
    }

    return body->eval(param_env);
}

Expr Letrec::eval(Assoc &env) {
    //TODO: To complete the letrec logic
}

Expr Set::eval(Assoc &env) {
    //TODO: To complete the set logic
}

Expr Display::evalRator(const Expr &rand) { // display function
    if (rand->e_type == E_STRING) {
        StringExpr* str_ptr = dynamic_cast<StringExpr*>(rand.get());
        std::cout << str_ptr->s;
    } else {
        rand->show(std::cout);
    }
    
    return MakeVoidE();
}