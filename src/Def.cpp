/**
 * @file Def.cpp
 * @brief Implementation of primitive functions and reserved words mappings
 * @author luke36
 * 
 * This file defines the mapping tables that associate Scheme function names
 * and special forms with their corresponding internal expression types.
 */

#include "Def.hpp"

/**
 * @brief Mapping of primitive function names to expression types
 * 
 * This map contains all built-in functions that can be called in Scheme.
 * These are functions that have direct implementations in the interpreter
 * and can be used in function application contexts.
 * 
 * Categories:
 * - Arithmetic: +, -, *, /, modulo, expt
 * - Comparison: <, <=, =, >=, >
 * - List operations: cons, car, cdr, list, set-car!, set-cdr!
 * - Logic: not, and, or (and/or support short-circuit evaluation)
 * - Type predicates: eq?, boolean?, number?, null?, pair?, procedure?, symbol?, list?, string?
 * - I/O: display
 * - Control: void, exit
 */
std::unordered_map<std::string, ExprType> primitives = {
    // Arithmetic operations
    {"+",        E_PLUS},
    {"-",        E_MINUS},
    {"*",        E_MUL},
    {"/",        E_DIV},
    {"modulo",   E_MODULO},
    {"expt",     E_EXPT},
    
    // Comparison operations
    {"<",        E_LT},
    {"<=",       E_LE},
    {"=",        E_EQ},
    {">=",       E_GE},
    {">",        E_GT},

     // List operations
    {"cons",      E_CONS},
    {"car",       E_CAR},
    {"cdr",       E_CDR},
    {"list",      E_LIST},
    {"set-car!",  E_SETCAR},
    {"set-cdr!",  E_SETCDR},

    // Logic operations
    {"not",       E_NOT},
    {"and",       E_AND},
    {"or",        E_OR},
    
    // Type predicates
    {"eq?",        E_EQQ},
    {"boolean?",   E_BOOLQ},
    {"number?",    E_INTQ},      
    {"null?",      E_NULLQ},
    {"pair?",      E_PAIRQ},
    {"procedure?", E_PROCQ},
    {"symbol?",    E_SYMBOLQ},
    {"list?",      E_LISTQ},
    {"string?",    E_STRINGQ},
    
    // I/O operations
    {"display",   E_DISPLAY},
    
    // Special values and control
    {"void",      E_VOID},
    {"exit",      E_EXIT}
};

/**
 * @brief Mapping of reserved words (special forms) to expression types
 * 
 * This map contains Scheme special forms that have special syntax and
 * evaluation rules. These cannot be used as regular function names and
 * have special parsing and evaluation semantics.
 * 
 * Categories:
 * - Control flow constructs: begin, quote
 * - Conditional : if, cond
 * - Function definition: lambda
 * - Variable and function definition: define
 * - Binding constructs: let, letrec
 * - Assignment: set!
 * 
 * Note: and/or have been moved to primitives to support function-style usage
 * while maintaining their short-circuit evaluation behavior.
 */
std::unordered_map<std::string, ExprType> reserved_words = {
    // Control flow constructs
    {"begin",   E_BEGIN},    
    {"quote",   E_QUOTE},    

    // Conditional
    {"if",      E_IF},       
    {"cond",    E_COND},     

    // Function definition
    {"lambda",  E_LAMBDA},   

    // Variable and function definition
    {"define",  E_DEFINE},   

    // Binding constructs
    {"let",     E_LET},      
    {"letrec",  E_LETREC},   
    
    // Assignment
    {"set!",    E_SET}      
};
