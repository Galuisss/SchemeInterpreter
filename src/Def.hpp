#ifndef DEF_HPP
#define DEF_HPP

/**
 * @file Def.hpp
 * @brief Core definitions and enumerations for the Scheme interpreter
 * @author luke36
 * 
 * This file contains essential type definitions, enumerations, and forward
 * declarations used throughout the Scheme interpreter implementation.
 */

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>

// Forward declarations
struct Syntax;
struct Expr;
struct Env;
using EnvPtr = std::shared_ptr<Env>;

/**
 * @brief Expression types enumeration
 * 
 * Defines all possible expression types that can be parsed and evaluated
 * in the Scheme interpreter.
 */
enum ExprType {
    // Basic types and literals
    E_FIXNUM,          
    E_RATIONAL,        
    E_STRING,
    E_BOOLEAN,               
    E_VOID,          
    E_EXIT,
    E_NULL,
    E_PROC,
    E_PAIR,
    E_PRIMITIVE,
    E_SPECIALFORM,
    E_EMPTY,            

    // Arithmetic operations
    E_PLUS,
    E_MINUS,
    E_MUL,
    E_DIV,
    E_MODULO,
    E_EXPT,

    // Comparison operations
    E_LT,              
    E_LE,             
    E_EQ,             
    E_GE,             
    E_GT,             

    // List operations
    E_CONS,             
    E_CAR,             
    E_CDR,             
    E_LIST,             
    E_SETCAR,          
    E_SETCDR,          

    // Logic operations
    E_NOT,              
    E_AND,             
    E_OR,
    
    // Type predicates
    E_EQQ,              
    E_BOOLQ,           
    E_INTQ,            
    E_NULLQ,            
    E_PAIRQ,            
    E_PROCQ,           
    E_SYMBOLQ,         
    E_LISTQ,                
    E_STRINGQ,          

    // Control flow constructs
    E_BEGIN,          
    E_QUOTE,          

    //Conditional
    E_IF,             
    E_COND,            

    // Variables and function definition
    E_VAR,
    E_SLIST,              
    E_APPLY,           
    E_LAMBDA,         
    E_DEFINE,          

    // Binding constructs
    E_LET,            
    E_LETREC,          

    // Assignment
    E_SET,             

    // I/O operations
    E_DISPLAY,
};

#endif // DEF_HPP
