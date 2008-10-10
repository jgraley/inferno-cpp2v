#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APInt.h"

#include <string>
#include <vector>

template<typename ELEMENT>
struct Sequence : public std::vector< RCPtr<ELEMENT> >
{
};                   

struct Node : public clang::SourceLocation
{               
    virtual ~Node(){}  
};

struct Identifier : public Node,
                    public std::string
{
};

// ProgramElement is the base class for Statement and Declarator. There's 
// nothing in Clang or C++ BNF for this, but we need it to avoid 
// a multiple inheritance diamond because certain decls (eg int a;)
// can appear at top level and in fn bodies. 
struct ProgramElement : public Node
{
};

struct Declarator : public ProgramElement
{   
    RCPtr<Identifier> identifier;
};

struct Scope : public Node,
               public Sequence<ProgramElement>
{
};                   

struct Type : public Node
{
};

struct VariableDeclarator : public Declarator
{
    RCPtr<Type> type;
    enum
    {
        DEFAULT,
        STATIC,
        AUTO
    } storage_class;
};

struct FunctionDeclarator : public Declarator
{
    RCPtr<Type> return_type;
    Sequence<VariableDeclarator> parameters;
    RCPtr<Scope> body;
};

struct Program : public Scope
{
};

struct Int : public Type
{
};

struct Char : public Type
{
};

struct Void : public Type
{
};

struct Expression : public Node
{
};

struct IdentifierExpression : public Expression
{
    RCPtr<Identifier> identifier;
};

struct Operator : public Expression
{
    Sequence<Expression> operands;
    clang::tok::TokenKind kind;
};

struct Prefix : public Operator
{
};

struct Postfix : public Operator
{
};

struct Infix : public Operator
{
};

struct ConditionalOperator : public Expression // eg ?:
{
    RCPtr<Expression> condition;
    RCPtr<Expression> if_true;
    RCPtr<Expression> if_false;
};

struct Statement : public ProgramElement
{
};

struct ExpressionStatement : public Statement
{
    RCPtr<Expression> expression;
};

struct Return : public Statement
{
    RCPtr<Expression> return_value;
};

struct NumericConstant : public Expression,
                         public llvm::APInt
{
};

#endif
