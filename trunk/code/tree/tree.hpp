#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APInt.h"

#include <string>
#include <vector>

template<typename ELEMENT>
struct Sequence : public vector< shared_ptr<ELEMENT> >
{
};                   

struct Node
{               
    virtual ~Node(){}  // be a virtual hierarchy
};

struct Identifier : public Node,
                    public string
{
};

// ProgramElement is the base class for Statement and Declarator. There's 
// nothing in Clang or C++ BNF for this, but we need it to avoid 
// a multiple inheritance diamond because certain decls (eg int a;)
// can appear at top level and in fn bodies. 
struct ProgramElement : public Node
{
};

struct Statement : public ProgramElement
{
};

struct Expression : public Statement
{
};

struct Scope : public Expression,
               public Sequence<ProgramElement>
{
};                   

struct Type : public Node
{
};

struct Declarator : public ProgramElement
{   
    enum
    {
        DEFAULT,
        STATIC,
        AUTO
    } storage_class;
    shared_ptr<Type> type;
    shared_ptr<Identifier> identifier; // NULL if anonymous
    shared_ptr<Expression> initialiser; // NULL if uninitialised
};

struct VariableDeclarator : public Declarator
{
};

struct FunctionPrototype : public Type
{
    shared_ptr<Type> return_type;
    Sequence<VariableDeclarator> parameters;
};

struct FunctionDeclarator : public Declarator
{
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

struct IdentifierExpression : public Expression
{
    shared_ptr<Identifier> identifier;
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
    shared_ptr<Expression> condition;
    shared_ptr<Expression> if_true;
    shared_ptr<Expression> if_false;
};

struct Return : public Statement
{
    shared_ptr<Expression> return_value;
};

struct Call : public Expression
{
    shared_ptr<Expression> function;
    Sequence<Expression> arguments;
};

struct NumericConstant : public Expression,
                         public llvm::APInt
{
};

#endif
