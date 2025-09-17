#ifndef EXPRESSION1
#define EXPRESSION1
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
#endif