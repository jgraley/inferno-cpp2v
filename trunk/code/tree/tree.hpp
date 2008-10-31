#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APInt.h"

#include <string>
#include <deque>

template<typename ELEMENT>
struct Sequence : deque< shared_ptr<ELEMENT> >
{
};                   

struct Node
{               
    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds 
    // without making Node ambiguous
};

struct Statement : virtual Node
{
};

struct Expression : Statement
{
};

struct Declaration : Statement
{   
};

struct Type : virtual Node
{
};

struct Identifier : Expression
{
    string identifier;
};

// A type that the user has created, and hence has a name. 
// These can be linked directly from a delaration list to 
// indicate their declaration (no seperate declaration node required).
struct UserType : Type,
                  Declaration
{
    string identifier;
};

struct Typedef : UserType
{
    shared_ptr<Type> type;
}; 

struct Label : Identifier
{
};

// Note that an object can be a function instance (ie the target
// of a function pointer) as well as a class instance or variable
struct Object : Identifier
{
    enum StorageClass
    {
        DEFAULT, // TODO get rid of default and fill in properly
        STATIC,
        AUTO
    } storage_class;
    enum Access
    {
        PUBLIC,
        PRIVATE,
        PROTECTED
    } access;
    shared_ptr<Type> type;
};

struct Scope : Expression,
               Sequence<Statement>
{
};                   

struct ObjectDeclaration : Declaration
{
    shared_ptr<Object> object; 
    shared_ptr<Expression> initialiser; // NULL if uninitialised
};

struct Function : Type
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

struct Program : Sequence<Declaration>
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

struct Holder : UserType
{
    Sequence<Declaration> members;
};

struct Union : Holder
{
};

struct InheritanceHolder : Holder
{
    Sequence<Declaration> bases; // these have empty identifier and NULL initialiser
};

struct Struct : InheritanceHolder
{
};

struct Class : InheritanceHolder
{
};

#endif
