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

struct Statement : public Node
{
};

struct Expression : public Statement
{
};

struct Identifier : public Expression,
                    public string
{
};

struct Label : Identifier
{
};

struct Variable : Identifier
{
};

struct Scope : public Expression,
               public Sequence<Statement>
{
};                   

struct Type : public Node
{
};

struct Declaration : public Statement
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

struct VariableDeclaration : public Declaration
{
};

struct FunctionPrototype : public Type
{
    shared_ptr<Type> return_type;
    Sequence<VariableDeclaration> parameters;
};

struct Pointer : public Type
{
    shared_ptr<Type> destination;
};

struct Reference : public Type // TODO could ref derive from ptr?
{
    shared_ptr<Type> destination;
};

struct FunctionDeclaration : public Declaration
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

struct LabelMarker : public Statement
{
    shared_ptr<Identifier> identifier;
};

struct Goto : public Statement
{
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have IdentifierExpression here.
    shared_ptr<Expression> destination;
};

struct If : public Statement
{
    shared_ptr<Expression> condition;
    shared_ptr<Statement> body;
    shared_ptr<Statement> else_body; // can be NULL if no else clause
};

struct Loop : public Statement
{
    shared_ptr<Statement> body;
};

struct While : public Loop
{
    shared_ptr<Expression> condition;
};

struct Do : public Loop // a do..while() construct 
{
    shared_ptr<Expression> condition;
};

struct For : Loop
{
    // Any of these can be NULL if absent. NULL condition evaluates true.
    shared_ptr<Statement> initialisation;
    shared_ptr<Expression> condition;
    shared_ptr<Statement> increment;    
};

#endif
