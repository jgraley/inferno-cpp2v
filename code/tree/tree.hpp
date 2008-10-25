#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APInt.h"

#include <string>
#include <vector>

template<typename ELEMENT>
struct Sequence : vector< shared_ptr<ELEMENT> >
{
};                   

struct Node
{               
    virtual ~Node(){}  // be a virtual hierarchy
};

struct Statement : Node
{
};

struct Type : Node
{
};

struct Expression : Statement
{
};

struct Identifier : Expression
{
    string identifier;
};

struct Label : Identifier
{
};

struct Object : Identifier
{
    enum StorageClass
    {
        DEFAULT, // TODO get rid of default and fill in properly
        STATIC,
        AUTO
    } storage_class;
    shared_ptr<Type> type;
};

struct Scope : Expression,
               Sequence<Statement>
{
};                   

struct Declaration : Statement
{   
    shared_ptr<Object> object; 
    shared_ptr<Expression> initialiser; // NULL if uninitialised
};

struct ObjectDeclaration : Declaration
{
};

struct FunctionPrototype : Type
{
    shared_ptr<Type> return_type;
    Sequence<ObjectDeclaration> parameters;
};

struct Pointer : Type
{
    shared_ptr<Type> destination;
};

struct Reference : Type // TODO could ref derive from ptr?
{
    shared_ptr<Type> destination;
};

struct FunctionDeclaration : Declaration
{
};

struct Program : Scope
{
};

struct Int : Type
{
};

struct Char : Type
{
};

struct Void : Type
{
};

struct Operator : Expression
{
    Sequence<Expression> operands;
    clang::tok::TokenKind kind;
};

struct Prefix : Operator
{
};

struct Postfix : Operator
{
};

struct Infix : Operator
{
};

struct ConditionalOperator : Expression // eg ?:
{
    shared_ptr<Expression> condition;
    shared_ptr<Expression> if_true;
    shared_ptr<Expression> if_false;
};

struct Return : Statement
{
    shared_ptr<Expression> return_value;
};

struct Call : Expression
{
    shared_ptr<Expression> function;
    Sequence<Expression> arguments;
};

struct NumericConstant : Expression,
                          llvm::APInt
{
};

struct LabelMarker : Statement
{
    shared_ptr<Label> label; // TODO these should be function scope
};

struct Goto : Statement
{
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have IdentifierExpression here.
    shared_ptr<Expression> destination;
};

struct If : Statement
{
    shared_ptr<Expression> condition;
    shared_ptr<Statement> body;
    shared_ptr<Statement> else_body; // can be NULL if no else clause
};

struct Loop : Statement
{
    shared_ptr<Statement> body;
};

struct While : Loop
{
    shared_ptr<Expression> condition;
};

struct Do : Loop // a do..while() construct 
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

struct Switch : Statement
{
    shared_ptr<Expression> condition;
    shared_ptr<Statement> body;
};

struct SwitchMarker : Statement
{
};

struct Case : SwitchMarker
{
    shared_ptr<Expression> value;
};

struct Default : SwitchMarker
{
};

struct Continue : Statement
{
};

struct Break : Statement
{
};

#endif
