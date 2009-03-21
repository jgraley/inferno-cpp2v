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
#include "common/itemise_members.hpp"

template<typename ELEMENT>
struct Sequence : Itemiser::Itemisable< deque< shared_ptr<ELEMENT> > > {};                   

template<typename ELEMENT>
struct SharedPtr : Itemiser::Itemisable< shared_ptr<ELEMENT> > 
{
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        Itemiser::Itemisable< shared_ptr<ELEMENT> >( shared_ptr<ELEMENT>(o) ) {}
    SharedPtr() {}
};           

//////////////////////////// Node Model ////////////////////////////

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

//////////////////////////// Properties ///////////////////////////////
// TODO seperate source file

struct Property : Node {};

// Means can be used as a literal
struct FundamentalProperty : Property {};

struct AnyString : FundamentalProperty {};
struct String : AnyString
{
    string value;
};

struct AnyNumber : FundamentalProperty {};

struct AnyInteger : AnyNumber {};
struct Integer : AnyInteger
{
    llvm::APSInt value; // APSint can be signed or unsigned
};

struct AnyFloat : AnyNumber {};
struct Float : AnyFloat
{
    Float( llvm::APFloat v ) : value(v) {};
    llvm::APFloat value; 
};

//////////////////////////// Underlying Program Nodes ////////////////////////////

struct Statement : virtual Node {};

struct Operand : virtual Node {};

struct Type : virtual Operand {};

struct Expression : Statement,
                    Operand {};
    
struct AccessSpec : Property {};
struct Public : AccessSpec {};
struct Private : AccessSpec {};
struct Protected : AccessSpec {};

struct Declaration : Statement
{   
    SharedPtr<AccessSpec> access;
};

struct Program : Node,
                 Sequence<Declaration>
{
};

//////////////////////////// Declarations /////////////////////

struct Identifier : Declaration
{
    string name;
};

struct AnyVirtual : Property {};
struct Virtual : AnyVirtual 
{
    // TODO pure
};
struct NonVirtual : AnyVirtual {};

struct StorageClass : Property {};
struct Static : StorageClass {};
struct NonStatic : StorageClass 
{
    SharedPtr<AnyVirtual> virt;
};

struct AnyConst : Property {};
struct Const : AnyConst {};
struct NonConst : AnyConst {};

struct Physical
{
    SharedPtr<StorageClass> storage;
    SharedPtr<AnyConst> constant; // TODO all functions to be const (otherwise would imply self-modifiying code). See idempotent
};

// can be an object or a function. In case of function, type is a type under Subroutine
struct Instance : Identifier,
                  Operand,
                  Physical
{
    SharedPtr<Type> type;
    SharedPtr<Operand> initialiser; // NULL if uninitialised
};

struct InheritanceRecord;
struct Base : Declaration
{
    SharedPtr<InheritanceRecord> record;
    // TODO virtual inheritance, treat seperately from virtual members
    // since different thing.
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
    SharedPtr<Type> return_type;
};

// The init list is just 0 or more Invoke( member, c'tor, params ) 
// in the body
struct Constructor : Procedure {};

struct Destructor : Subroutine {};

struct Pointer : Type
{
    SharedPtr<Type> destination;
};

struct Reference : Type // TODO could ref derive from ptr?
{
    SharedPtr<Type> destination;
};

struct Void : Type {};

struct Bool : Type {};

struct Numeric : Type 
{
    SharedPtr<Operand> width;  // Bits, not bytes
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
    SharedPtr<Type> type;
}; 

struct AnyComplete : Property {};
struct Complete : AnyComplete {};
struct Incomplete : AnyComplete {};

struct Record : UserType
{
    Sequence<Declaration> members;
    
    // Where eg struct foo; is used we should create seperate nodes for
    // the incomplete and complete types. This is so that mutually 
    // referencing structs don't create a loop in the tree
    // (but we could use weak_ptr<> for such refs?).
    SharedPtr<AnyComplete> complete; 
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
    SharedPtr<Type> element;
    SharedPtr<Operand> size; // NULL if undefined
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

struct AnyAssignment : Property {};
struct Assignment : AnyAssignment {};
struct NonAssignment : AnyAssignment {};

struct Operator : Aggregate
{
    SharedPtr<AnyAssignment> assign; // write result back to left
};

struct Boolean : Operator {};
struct Bitwise : Boolean {};
struct Logical : Boolean {};
struct Arithmetic : Operator {};
struct Shift : Operator {};
struct Comparison : Operator {};

#define PREFIX(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE {};
#define POSTFIX(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE {};
#define BINARY(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE {};
#include "operator_info.inc"

struct SizeOf : Operator {};
struct AlignOf : Operator {};

struct ConditionalOperator : Expression // eg ?:
{
    SharedPtr<Operand> condition;
    SharedPtr<Operand> if_true;
    SharedPtr<Operand> if_false;
};

struct Call : Aggregate 
{
    SharedPtr<Operand> function;
};

struct AnyGlobalNew : Property {};
struct GlobalNew : AnyGlobalNew {}; // ::new/::delete was used
struct NonGlobalNew : AnyGlobalNew {}; // new/delete, no ::

struct AnyArrayNew : Property {};
struct ArrayNew : AnyArrayNew {}; // delete[]
struct NonArrayNew : AnyArrayNew {}; 

struct New : Expression
{
    SharedPtr<Type> type; 
    Sequence<Operand> placement_arguments;
    Sequence<Operand> constructor_arguments;
    SharedPtr<AnyGlobalNew> global;
};

struct Delete : Expression
{
    SharedPtr<Operand> pointer;
    SharedPtr<AnyArrayNew> array;
    SharedPtr<AnyGlobalNew> global;
};

struct Literal : Expression
{
    // Use properties to express value, to avoid duplication
    SharedPtr<FundamentalProperty> value;
};

struct This : Expression {};

struct Subscript : Expression // TODO could be an Operator?
{
    SharedPtr<Operand> base;
    SharedPtr<Operand> index;
};

struct Lookup : Expression  
{
    SharedPtr<Operand> base; 
    SharedPtr<Instance> member;    
};

struct Cast : Expression
{
    SharedPtr<Operand> operand;
    SharedPtr<Type> type;        
};

//////////////////////////// Statements ////////////////////////////

struct Return : Statement
{
    SharedPtr<Operand> return_value;
};

struct LabelTarget : Statement
{
    SharedPtr<Label> label; // TODO these should be function scope
};

struct Goto : Statement
{
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
    SharedPtr<Operand> destination;
};

struct If : Statement
{
    SharedPtr<Operand> condition;
    SharedPtr<Statement> body;
    SharedPtr<Statement> else_body; // can be NULL if no else clause
};

struct Loop : Statement
{
    SharedPtr<Statement> body;
};

struct While : Loop
{
    SharedPtr<Operand> condition;
};

struct Do : Loop // a do..while() construct 
{
    SharedPtr<Operand> condition;
};

struct For : Loop
{
    // Any of these can be NULL if absent. NULL condition evaluates true.
    SharedPtr<Statement> initialisation;
    SharedPtr<Operand> condition;
    SharedPtr<Statement> increment;    
};

struct Switch : Statement
{
    SharedPtr<Operand> condition;
    SharedPtr<Statement> body;
};

struct SwitchTarget : Statement {};

struct Case : SwitchTarget 
{
    // support gcc extension of case x..y:
    // in other cases, value_lo==value_hi
    SharedPtr<Operand> value_lo; // inclusive
    SharedPtr<Operand> value_hi; // inclusive
};

struct Default : SwitchTarget {};

struct Continue : Statement {};

struct Break : Statement {};

struct Nop : Statement {};

#endif
