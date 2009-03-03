#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include "common/magic.hpp"
#include <string>
#include <deque>

template<typename ELEMENT>
struct Sequence : deque< shared_ptr<ELEMENT> > {};                   

//////////////////////////// Node Models ////////////////////////////

struct Node : Magic
{               
    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds 
    // without making Node ambiguous
    
    virtual bool operator==( const Node &other )
    {
        return typeid( *this ) == typeid( other );
    }
    virtual bool operator!=( const Node &other )
    {
        return !(*this==other);
    }
};

struct Hard : Node {};

struct Soft : Node {};

//////////////////////////// Properties ///////////////////////////////
// TODO seperate source file

struct Property : Hard {};

struct Access : Property {};
struct Public : Access {};
struct Private : Access {};
struct Protected : Access {};

struct AnyConst : Property {};
struct Const : AnyConst {};
struct NonConst : AnyConst {};

//////////////////////////// Underlying Program Nodes ////////////////////////////

struct Type : virtual Hard {};

struct Statement : virtual Hard {};

struct Operand : virtual Hard {};

struct Expression : Statement,
                    Operand {};
    
struct Declaration : Statement
{   
    shared_ptr<Access> access;
};

struct Program : Hard,
                 Sequence<Declaration>
{
};

//////////////////////////// Declarations /////////////////////

struct Identifier : Declaration
{
    string name;
};

enum Storage
{
    STATIC,
    DEFAULT, 
    VIRTUAL // implies DEFAULT
    //PURE // implies VIRTUAL TODO implment in parse and render
};

struct Physical
{
    Storage storage;
    shared_ptr<AnyConst> constant; // TODO all functions to be const (otherwise would imply self-modifiying code). See idempotent
};

// can be an object or a function. In case of function, type is a type under Subroutine
struct Instance : Identifier,
                  Operand,
                  Physical
{
    shared_ptr<Type> type;
    shared_ptr<Operand> initialiser; // NULL if uninitialised
};

struct InheritanceRecord;
struct Base : Declaration,
              Physical
{
    shared_ptr<InheritanceRecord> record;
};              

//////////////////////////// Anonymous Types ////////////////////////////

// Subroutine like in Basic, no params or return.
// The type refers to the interface as seen by caller - you need
// an & before to have a "function pointer"
struct Subroutine : Type 
{
    // TODO add bool idempotent; here for member functions with "const" at the end of the decl.
};

// Like in pascal etc, params but no return value
struct Procedure : Subroutine
{
    Sequence<Instance> parameters;
};

// Like in C, Pascal; params and a single return value
struct Function : Procedure
{
    shared_ptr<Type> return_type;
};

// The init list is just 0 or more Invoke( member, c'tor, params ) 
// in the body
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

struct Void : Type {};

struct Bool : Type {};

struct Numeric : Type 
{
    shared_ptr<Operand> width;  // Bits, not bytes
};

struct Integral : Numeric {};

struct Signed : Integral {};

struct Unsigned : Integral {};

struct Floating : Numeric {}; // Note width determines float vs double 

//////////////////////////// User-defined Types ////////////////////////////

// A type that the user has created, and hence has a name. 
// These can be linked directly from a Sequence<> to indicate 
// their declaration (no seperate declaration node required).
struct UserType : Type,
                  Identifier {};

struct Typedef : UserType
{
    shared_ptr<Type> type;
}; 

struct Record : UserType
{
    Sequence<Declaration> members;
    
    // Where eg struct foo; is used we should create seperate nodes for
    // the incomplete and complete types. This is so that mutually 
    // referencing structs don't create a loop in the tree.
    // TODO use weak_ptr for access to structs
    bool incomplete; 
};

struct Union : Record {};

struct Enum : Record {};

struct InheritanceRecord : Record // TODO InheritanceRecord
{
    Sequence<Base> bases; // these have empty identifier and NULL initialiser
};

struct Struct : InheritanceRecord {};

struct Class : InheritanceRecord {};

struct Array : Type
{
    shared_ptr<Type> element;
    shared_ptr<Operand> size; // NULL if undefined
};

//////////////////////////// Expressions ////////////////////////////

// TODO consider making this an object, STATIC and void * type
struct Label : Identifier,
               Expression {}; 

// The result of a Compound, if viewed as an Operand, is the
// code itself (imagine a generic byte code) not the result of
// execution. You have to do Call or Invoke on it to get 
// the execution result.
struct Compound : Expression
{
    Sequence<Statement> statements;
};                   

struct Aggregate : Expression
{
    Sequence<Operand> operands; 
};

struct Operator : Aggregate
{
    clang::tok::TokenKind kind;
};

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
    shared_ptr<Operand> condition;
    shared_ptr<Operand> if_true;
    shared_ptr<Operand> if_false;
};

struct Call : Aggregate 
{
    shared_ptr<Operand> function;
};

struct New : Expression
{
    shared_ptr<Type> type; 
    Sequence<Operand> placement_arguments;
    Sequence<Operand> constructor_arguments;
    bool global; // true if ::new was used
};

struct Delete : Expression
{
    shared_ptr<Operand> pointer;
    bool array; // true if delete[] was used
    bool global; // true if ::delete was used
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

struct Subscript : Expression // TODO could be an Operator?
{
    shared_ptr<Operand> base;
    shared_ptr<Operand> index;
};

struct Lookup : Expression  
{
    shared_ptr<Operand> base; 
    shared_ptr<Instance> member;    
};

struct Cast : Expression
{
    shared_ptr<Operand> operand;
    shared_ptr<Type> type;        
};

//////////////////////////// Statements ////////////////////////////

struct Return : Statement
{
    shared_ptr<Operand> return_value;
};

struct LabelTarget : Statement
{
    shared_ptr<Label> label; // TODO these should be function scope
};

struct Goto : Statement
{
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
    shared_ptr<Operand> destination;
};

struct If : Statement
{
    shared_ptr<Operand> condition;
    shared_ptr<Statement> body;
    shared_ptr<Statement> else_body; // can be NULL if no else clause
};

struct Loop : Statement
{
    shared_ptr<Statement> body;
};

struct While : Loop
{
    shared_ptr<Operand> condition;
};

struct Do : Loop // a do..while() construct 
{
    shared_ptr<Operand> condition;
};

struct For : Loop
{
    // Any of these can be NULL if absent. NULL condition evaluates true.
    shared_ptr<Statement> initialisation;
    shared_ptr<Operand> condition;
    shared_ptr<Statement> increment;    
};

struct Switch : Statement
{
    shared_ptr<Operand> condition;
    shared_ptr<Statement> body;
};

struct SwitchTarget : Statement {};

struct Case : SwitchTarget 
{
    // support gcc extension of case x..y:
    // in other cases, value_lo==value_hi
    shared_ptr<Operand> value_lo; // inclusive
    shared_ptr<Operand> value_hi; // inclusive
};

struct Default : SwitchTarget {};

struct Continue : Statement {};

struct Break : Statement {};

struct Nop : Statement {};

#endif
