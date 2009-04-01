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

struct Node;

struct GenericSequence : Itemiser::Itemisable
{
    virtual shared_ptr<Node> Get(int i) = 0;
    virtual int size() const = 0;
};

template<typename ELEMENT>
struct Sequence : GenericSequence, deque< shared_ptr<ELEMENT> > 
{
    virtual shared_ptr<Node> Get(int i)
    {
        return (shared_ptr<Node>)(*(deque< shared_ptr<ELEMENT> > *)this)[i];
    }
    virtual int size() const
    {
        return ((deque< shared_ptr<ELEMENT> > *)this)->size();
    }
};

struct GenericPointer : Itemiser::Itemisable
{
    virtual shared_ptr<Node> Get() = 0;
};

template<typename ELEMENT>
struct SharedPtr : GenericPointer, shared_ptr<ELEMENT> 
{
    virtual shared_ptr<Node> Get()
    {
        return (shared_ptr<Node>)*(shared_ptr<ELEMENT> *)this;
    }
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>(o) {}
    SharedPtr() {}
};           

#define NODE_FUNCTIONS ITEMISE_FUNCTION

//////////////////////////// Node Model ////////////////////////////

struct Node : Magic
{            
    NODE_FUNCTIONS
   
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

struct Property : Node { NODE_FUNCTIONS };

// Means can be used as a literal
struct FundamentalProperty : Property { NODE_FUNCTIONS };

struct AnyString : FundamentalProperty { NODE_FUNCTIONS };
struct String : AnyString
{
    NODE_FUNCTIONS
    string value;
};

struct AnyNumber : FundamentalProperty { NODE_FUNCTIONS };

struct AnyInteger : AnyNumber { NODE_FUNCTIONS };
struct Integer : AnyInteger
{
    NODE_FUNCTIONS
    llvm::APSInt value; // APSint can be signed or unsigned
};

struct AnyFloat : AnyNumber { NODE_FUNCTIONS };
struct Float : AnyFloat
{
    NODE_FUNCTIONS
    Float() : value((float)0) {};
    Float( llvm::APFloat v ) : value(v) {};
    llvm::APFloat value; 
};

//////////////////////////// Underlying Program Nodes ////////////////////////////

struct Statement : virtual Node { NODE_FUNCTIONS };

struct Operand : virtual Node { NODE_FUNCTIONS };

struct Type : virtual Operand { NODE_FUNCTIONS };

struct Expression : Statement,
                    Operand { NODE_FUNCTIONS };
    
struct AccessSpec : Property { NODE_FUNCTIONS };
struct Public : AccessSpec { NODE_FUNCTIONS };
struct Private : AccessSpec { NODE_FUNCTIONS };
struct Protected : AccessSpec { NODE_FUNCTIONS };

struct Declaration : Statement
{   
    NODE_FUNCTIONS
    SharedPtr<AccessSpec> access;
};

struct Program : Node,
                 Sequence<Declaration>
{
    NODE_FUNCTIONS
};

//////////////////////////// Declarations /////////////////////

struct Identifier : virtual Node { NODE_FUNCTIONS };

struct AnyInstanceIdentifier : Identifier,
                               Operand { NODE_FUNCTIONS };
struct InstanceIdentifier : AnyInstanceIdentifier, 
                            String { NODE_FUNCTIONS };

struct AnyTypeIdentifier : Identifier,
                           Type { NODE_FUNCTIONS };
struct TypeIdentifier : AnyTypeIdentifier,
                        String { NODE_FUNCTIONS };

struct AnyLabelIdentifier : Identifier,
                            Expression { NODE_FUNCTIONS };
struct LabelIdentifier : AnyLabelIdentifier,
                         String { NODE_FUNCTIONS };

struct AnyVirtual : Property { NODE_FUNCTIONS };
struct Virtual : AnyVirtual 
{
    NODE_FUNCTIONS
    // TODO pure
};
struct NonVirtual : AnyVirtual { NODE_FUNCTIONS };

struct StorageClass : Property { NODE_FUNCTIONS };
struct Static : StorageClass { NODE_FUNCTIONS };
struct NonStatic : StorageClass 
{
    NODE_FUNCTIONS
    SharedPtr<AnyVirtual> virt;
};

struct AnyConst : Property { NODE_FUNCTIONS };
struct Const : AnyConst { NODE_FUNCTIONS };
struct NonConst : AnyConst { NODE_FUNCTIONS };

struct Physical
{
    NODE_FUNCTIONS
    SharedPtr<StorageClass> storage;
    SharedPtr<AnyConst> constant; // TODO all functions to be const (otherwise would imply self-modifiying code). See idempotent
};

// can be an object or a function. In case of function, type is a type under Subroutine
struct Instance : Declaration,
                  Physical
{
    NODE_FUNCTIONS
    SharedPtr<AnyInstanceIdentifier> identifier;
    SharedPtr<Type> type;
    SharedPtr<Operand> initialiser; // NULL if uninitialised
};

struct InheritanceRecord;
struct Base : Declaration
{
    NODE_FUNCTIONS
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
    NODE_FUNCTIONS
    // TODO add bool idempotent; here for member functions with "const" at the end of the decl.
};

// Like in pascal etc, params but no return value
struct Procedure : Subroutine
{
    NODE_FUNCTIONS
    Sequence<Instance> parameters;
};

// Like in C, Pascal; params and a single return value
struct Function : Procedure
{
    NODE_FUNCTIONS
    SharedPtr<Type> return_type;
};

// The init list is just 0 or more Invoke( member, c'tor, params ) 
// in the body
struct Constructor : Procedure { NODE_FUNCTIONS };

struct Destructor : Subroutine { NODE_FUNCTIONS };

struct Pointer : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> destination;
};

struct Reference : Type // TODO could ref derive from ptr?
{
    NODE_FUNCTIONS
    SharedPtr<Type> destination;
};

struct Void : Type { NODE_FUNCTIONS };

struct Bool : Type { NODE_FUNCTIONS };

struct Numeric : Type 
{
    NODE_FUNCTIONS
    SharedPtr<Operand> width;  // Bits, not bytes
};

struct Integral : Numeric { NODE_FUNCTIONS };

struct Signed : Integral { NODE_FUNCTIONS };

struct Unsigned : Integral { NODE_FUNCTIONS };

struct Floating : Numeric { NODE_FUNCTIONS }; // Note width determines float vs double 

//////////////////////////// User-defined Types ////////////////////////////

// A type that the user has created, and hence has a name. 
// These can be linked directly from a Sequence<> to indicate 
// their declaration (no seperate declaration node required).
struct UserType : Declaration 
{ 
    NODE_FUNCTIONS
    SharedPtr<AnyTypeIdentifier> identifier;
};

struct Typedef : UserType
{
    NODE_FUNCTIONS
    SharedPtr<Type> type;
}; 

struct AnyComplete : Property { NODE_FUNCTIONS };
struct Complete : AnyComplete { NODE_FUNCTIONS };
struct Incomplete : AnyComplete { NODE_FUNCTIONS };

struct Record : UserType
{
    NODE_FUNCTIONS
    Sequence<Declaration> members;
    
    // Where eg struct foo; is used we should create seperate nodes for
    // the incomplete and complete types. This is so that mutually 
    // referencing structs don't create a loop in the tree
    // (but we could use weak_ptr<> for such refs?).
    // TODO masked issue: classes can contain pointers to themselves
    SharedPtr<AnyComplete> complete; 
};

struct Union : Record { NODE_FUNCTIONS };

struct Enum : Record { NODE_FUNCTIONS };

struct InheritanceRecord : Record // TODO InheritanceRecord
{
    NODE_FUNCTIONS
    Sequence<Base> bases; // these have empty identifier and NULL initialiser
};

struct Struct : InheritanceRecord { NODE_FUNCTIONS };

struct Class : InheritanceRecord { NODE_FUNCTIONS };

struct Array : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> element;
    SharedPtr<Operand> size; // NULL if undefined
};

//////////////////////////// Expressions ////////////////////////////

// TODO consider making this an object, STATIC and void * type
struct Label : Declaration
{
    NODE_FUNCTIONS
    SharedPtr<AnyLabelIdentifier> identifier;
}; 

// The result of a Compound, if viewed as an Operand, is the
// code itself (imagine a generic byte code) not the result of
// execution. You have to do Call or Invoke on it to get 
// the execution result.
struct Compound : Expression
{
    NODE_FUNCTIONS
    Sequence<Statement> statements;
};                   

struct Aggregate : Expression
{
    NODE_FUNCTIONS
    Sequence<Operand> operands; 
};

struct AnyAssignment : Property { NODE_FUNCTIONS };
struct Assignment : AnyAssignment { NODE_FUNCTIONS };
struct NonAssignment : AnyAssignment { NODE_FUNCTIONS };

struct Operator : Aggregate
{
    NODE_FUNCTIONS
    SharedPtr<AnyAssignment> assign; // write result back to left
};

struct Boolean : Operator { NODE_FUNCTIONS };
struct Bitwise : Boolean { NODE_FUNCTIONS };
struct Logical : Boolean { NODE_FUNCTIONS };
struct Arithmetic : Operator { NODE_FUNCTIONS };
struct Shift : Operator { NODE_FUNCTIONS };
struct Comparison : Operator { NODE_FUNCTIONS };

#define PREFIX(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#define POSTFIX(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#define BINARY(TOK, TEXT, NODE, ASS, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#include "operator_info.inc"

struct SizeOf : Operator { NODE_FUNCTIONS };
struct AlignOf : Operator { NODE_FUNCTIONS };

struct ConditionalOperator : Expression // eg ?:
{
    NODE_FUNCTIONS
    SharedPtr<Operand> condition;
    SharedPtr<Operand> if_true;
    SharedPtr<Operand> if_false;
};

struct Call : Aggregate 
{
    NODE_FUNCTIONS
    SharedPtr<Operand> function;
};

struct AnyGlobalNew : Property { NODE_FUNCTIONS };
struct GlobalNew : AnyGlobalNew { NODE_FUNCTIONS }; // ::new/::delete was used
struct NonGlobalNew : AnyGlobalNew { NODE_FUNCTIONS }; // new/delete, no ::

struct AnyArrayNew : Property { NODE_FUNCTIONS };
struct ArrayNew : AnyArrayNew { NODE_FUNCTIONS }; // delete[]
struct NonArrayNew : AnyArrayNew { NODE_FUNCTIONS }; 

struct New : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Type> type; 
    Sequence<Operand> placement_arguments;
    Sequence<Operand> constructor_arguments;
    SharedPtr<AnyGlobalNew> global;
};

struct Delete : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Operand> pointer;
    SharedPtr<AnyArrayNew> array;
    SharedPtr<AnyGlobalNew> global;
};

struct Literal : Expression
{
    NODE_FUNCTIONS
    // Use properties to express value, to avoid duplication
    SharedPtr<FundamentalProperty> value;
};

struct This : Expression { NODE_FUNCTIONS };

struct Subscript : Expression // TODO could be an Operator?
{
    NODE_FUNCTIONS
    SharedPtr<Operand> base;
    SharedPtr<Operand> index;
};

struct Lookup : Expression  
{
    NODE_FUNCTIONS
    SharedPtr<Operand> base; 
    SharedPtr<AnyInstanceIdentifier> member;    
};

struct Cast : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Operand> operand;
    SharedPtr<Type> type;        
};

//////////////////////////// Statements ////////////////////////////

struct Return : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Operand> return_value;
};

struct LabelTarget : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Label> label; // TODO these should be function scope
};

struct Goto : Statement
{
    NODE_FUNCTIONS
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
    SharedPtr<Operand> destination;
};

struct If : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Operand> condition;
    SharedPtr<Statement> body;
    SharedPtr<Statement> else_body; // can be NULL if no else clause
};

struct Loop : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Statement> body;
};

struct While : Loop
{
    NODE_FUNCTIONS
    SharedPtr<Operand> condition;
};

struct Do : Loop // a do..while() construct 
{
    NODE_FUNCTIONS
    SharedPtr<Operand> condition;
};

struct For : Loop
{
    NODE_FUNCTIONS
    // Any of these can be NULL if absent. NULL condition evaluates true.
    SharedPtr<Statement> initialisation;
    SharedPtr<Operand> condition;
    SharedPtr<Statement> increment;    
};

struct Switch : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Operand> condition;
    SharedPtr<Statement> body;
};

struct SwitchTarget : Statement { NODE_FUNCTIONS };

struct Case : SwitchTarget 
{
    NODE_FUNCTIONS
    // support gcc extension of case x..y:
    // in other cases, value_lo==value_hi
    SharedPtr<Operand> value_lo; // inclusive
    SharedPtr<Operand> value_hi; // inclusive
};

struct Default : SwitchTarget { NODE_FUNCTIONS };

struct Continue : Statement { NODE_FUNCTIONS };

struct Break : Statement { NODE_FUNCTIONS };

struct Nop : Statement { NODE_FUNCTIONS };

#endif
