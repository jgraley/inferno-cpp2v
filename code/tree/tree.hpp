#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"

#include <string>
#include <deque>

template<typename ELEMENT>
struct Sequence : deque< shared_ptr<ELEMENT> > {};                   

struct Node
{               
    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds 
    // without making Node ambiguous
};

struct Statement : virtual Node {};

struct Expression : Statement {};

struct Declaration : Statement
{   
    enum Access
    {
        PUBLIC,
        PRIVATE,
        PROTECTED
    } access;
    bool is_virtual;
};

struct Type : virtual Node {};

struct Identifier
{
    string identifier;
    shared_ptr<Identifier> nested; // for foo::bar, this points to foo
    // nested==NULL means rooted in current scope; nested points to ""
    // means global scope (ie ::bar)
};

// A type that the user has created, and hence has a name. 
// These can be linked directly from a Sequence<> to indicate 
// their declaration (no seperate declaration node required).
struct UserType : Type,
                  Identifier,
                  Declaration {};

struct Typedef : UserType
{
    shared_ptr<Type> type;
}; 

// TODO consider making this an object, STATIC and void * type
struct Label : Identifier,
               Expression {}; 

// Note that an object can be a function instance (ie the target
// of a function pointer) as well as a class instance or variable.
// If it is a function, its value is the function implementation.
struct Object : Identifier,
                Expression
{
    enum Storage
    {
        DEFAULT, 
        STATIC,
        EXTERN,
        MEMBER,
        AUTO,
        SYMBOL // constant so no storage required
    } storage;
    shared_ptr<Type> type;
};

struct Compound : Expression 
{
    Sequence<Statement> statements;
};                   

struct InitCompound : Compound
{
    Sequence<Statement>  initialisers;
};                   

struct ObjectDeclaration : Declaration
{
    shared_ptr<Object> object; 
    shared_ptr<Expression> initialiser; // NULL if uninitialised
};

// Subroutine like in assembler, no params or return.
// The type refers to the interface as seen by caller
struct Subroutine : Type {};

// Like in pascal etc, params but no return value
struct Procedure : Subroutine
{
    Sequence<ObjectDeclaration> parameters;
};

struct Function : Procedure
{
    shared_ptr<Type> return_type;
};

struct Constructor : Procedure {};

struct Destructor : Subroutine {};

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

struct Void : Type {};

struct Bool : Type {};

struct Numeric : Type // TODO rename Numeric to Numeric, latter is more concise
{
    shared_ptr<Expression> width;  // Bits, not bytes
};

struct Integral : Numeric {};

struct Signed : Integral {};

struct Unsigned : Integral {};

struct Operator : Expression
{
    Sequence<Expression> operands;
    clang::tok::TokenKind kind;
};

struct Floating : Numeric {}; // Note width determines float vs double 

struct Prefix : Operator {};  // 1 operand (eg "++i" or "sizeof(i)")

struct Postfix : Operator {}; // 1 operand

struct Infix : Operator {}; // 2 operands

// for eg sizeof(int) where the operand is a type
struct PrefixOnType : Expression
{
    shared_ptr<Type> operand;
    clang::tok::TokenKind kind;
};

struct ConditionalOperator : Expression // eg ?:
{
    shared_ptr<Expression> condition;
    shared_ptr<Expression> if_true;
    shared_ptr<Expression> if_false;
};

struct Aggregate : Expression
{
    Sequence<Expression> elements;
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

struct NumericConstant : Expression {};

struct IntegralConstant : NumericConstant
{
    llvm::APSInt value; // APSint can be signed or unsigned
};

struct FloatingConstant : NumericConstant
{
    FloatingConstant( llvm::APFloat v ) : value(v) {};
    llvm::APFloat value; 
};

struct String : Expression
{
    string value;
};

struct This : Expression {};

struct LabelMarker : Statement
{
    shared_ptr<Label> label; // TODO these should be function scope
};

struct Goto : Statement
{
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
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

struct SwitchMarker : Statement {};

struct Case : SwitchMarker 
{
    // support gcc extension of case x..y:
    // in other cases, value_lo==value_hi
    shared_ptr<Expression> value_lo; // inclusive
    shared_ptr<Expression> value_hi; // inclusive
};

struct Default : SwitchMarker {};

struct Continue : Statement {};

struct Break : Statement {};

struct Holder : UserType
{
    Sequence<Declaration> members;
    
    // Where eg struct foo; is used we should create seperate nodes for
    // the incomplete and complete types. This is so that mutually 
    // referencing structs don't create a loop in the tree.
    bool incomplete; 
};

struct Union : Holder {};

struct InheritanceHolder : Holder
{
    Sequence<Declaration> bases; // these have empty identifier and NULL initialiser
};

struct Struct : InheritanceHolder {};

struct Class : InheritanceHolder {};

struct Array : Type
{
    shared_ptr<Type> element;
    shared_ptr<Expression> size; // NULL if undefined
};

struct Subscript : Expression
{
    shared_ptr<Expression> base;
    shared_ptr<Expression> index;
};

struct Access : Expression
{
    shared_ptr<Expression> base; 
    shared_ptr<Object> member;    
};

struct Enum : Holder {};

struct Cast : Expression
{
    shared_ptr<Expression> operand;
    shared_ptr<Type> type;        
};

struct Nop : Statement {};

#endif
