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
#include "itemise_members.hpp"
#include "type_info.hpp"
#include "duplicate.hpp"
#include "generics.hpp"

#define NODE_FUNCTIONS ITEMISE_FUNCTION TYPE_INFO_FUNCTION DUPLICATE_FUNCTION

//TODO all these in a namespace perhaps?

//////////////////////////// Node Model ////////////////////////////

// Base class for all tree nodes and nodes in search/replace 
// patterns etc. Convention is to use "struct" for derived
// node classes so that everything is public (inferno tree nodes
// are more like records in a database, they have only minimal 
// functionality). Also, all derived structs should contain the
// NODE_FUNCTIONS macro which expands to a few virtual functions
// required for common ("bounced") functionality. Where multiple
// inheritance dimaonds arise, Node should be derived virtually
// (we always want the set-restricting model of inheritance in
// the inferno tree node hierarchy).
struct Node : Magic, 
              TypeInfo::TypeBase, 
              Itemiser,
              Duplicator
{            
    NODE_FUNCTIONS
   
    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds 
    // without making Node ambiguous  
};

//////////////////////////// Underlying Node Types ////////////////////////////

// Nodes can be property nodes or topological nodes. Topological nodes
// represent parts of the program, and property nodes represent 
// ancilliary data (strings, numbers, enums). Property is the base
// class for property nodes (there is no base class for Topological 
// nodes, topological is assumed if not derivng from Property). 
// Enums are actually implemented by choosing one of a choice of
// empty node, not using enum. Each kind of property has an intermediate
// which can represent any value of the property - they have Any in 
// their name if there isn't a suitable language-specific name.
struct Property : virtual Node { NODE_FUNCTIONS };

// This intermediate is used for an initial value for for a variable/object in
// which case it will be an Expression, or for the implementation of a function
// in which case it will be a Statement. For an uninititialised variable/object
// or a function declaration, it will be Uninitialised.
struct Initialiser : virtual Node { NODE_FUNCTIONS };
struct Uninitialised : Initialiser { NODE_FUNCTIONS }; // an uninitialised Instance.

// Represents a statement as found inside a function definition. Basically anything 
// that ends with a ; inside a function body, as well as labels (which we consider as 
// statements in their own right).
struct Statement : Initialiser { NODE_FUNCTIONS };

// An expression that computes a result value. Can be used anywhere a statement 
// can, per C syntax rules.
struct Expression : Statement { NODE_FUNCTIONS };

// Any abstract data type including fundamentals, structs, function prototypes
// and user-named types. 
struct Type : virtual Node { NODE_FUNCTIONS };

// Property for C++ access speficiers public, protected and private. AccessSpec
// represents any access spec, the subsequent empty nodes specify particular
// access specs. Inferno uses access specs for all declarations. Declarations
// in classes are as specified, function parameters and external declarations
// are public, all others private. It is anticipated that the access spec will
// control generated high-level interfaces.
struct AccessSpec : Property { NODE_FUNCTIONS };
struct Public : AccessSpec { NODE_FUNCTIONS };
struct Private : AccessSpec { NODE_FUNCTIONS };
struct Protected : AccessSpec { NODE_FUNCTIONS };

// A declaration specifies the creation of a type or an object from a type. 
// We specify an access spec for all declarations and choose a default when
// the user cannot specify. Declaration can appear where statements can and
// also inside structs etc and at top level.
struct Declaration : Statement
{   
    NODE_FUNCTIONS
    SharedPtr<AccessSpec> access;
};

// The top level of a program is considered a sequence of declarations.
// main() would typically be a function instance somewhere in this sequence.
// TODO decide whether to allow/encourage making Sequence/SharedPtr be
// a base class - seems to work OK in sear/replace etc since itemise 
// doesn't differentiate.
struct Program : Node,
                 Sequence<Declaration>
{
    NODE_FUNCTIONS
};

//////////////////////////// Literals ///////////////////////////////

// Means a property that can be used as a literal in a program, so
// that we do not need to duplicate literals and proeprties.
struct Literal : Expression,
                 Property
{
    NODE_FUNCTIONS
};

// Intermediate property node that represents a string of any value.
struct AnyString : Literal { NODE_FUNCTIONS };

// A string with a particular value as specified. value must be filled
// in.
struct String : AnyString
{
    NODE_FUNCTIONS
    string value;
};

// Intermediate property node that represents a number (anything you
// can do +, - etc on) of any value.
struct AnyNumber : Literal { NODE_FUNCTIONS };

#define INTEGER_DEFAULT_WIDTH 32

// Intermediate property node that represents an integer number of any
// value (signed or unsigned).
struct AnyInteger : AnyNumber { NODE_FUNCTIONS };

// Property node for an integer number. We use LLVM's class for this, 
// so that we can deal with any size of number (so this can be used for
// large bit vectors). The LLVM object also stores the signedness. The
// value must always be filled in.
struct Integer : AnyInteger
{
    NODE_FUNCTIONS
    Integer( int i ) : value(INTEGER_DEFAULT_WIDTH) { value = i; }
    Integer() : value(INTEGER_DEFAULT_WIDTH) { value = 0; }
    llvm::APSInt value; // APSint can be signed or unsigned
};

// Intermediate property node that represents a floating point number of any
// value.
struct AnyFloat : AnyNumber { NODE_FUNCTIONS };

// Property node for an floating point number. We use LLVM's class for this, 
// so that we can deal with any representation convention. The value must 
// always be filled in. To determine the type, use llvm::getSemantics()
struct Float : AnyFloat
{
    NODE_FUNCTIONS
    Float() : value((float)0) {};
    Float( llvm::APFloat v ) : value(v) {};
    llvm::APFloat value; 
};

// Intermediate property node that represents any boolean value.
struct AnyBoolean : Literal { NODE_FUNCTIONS };

// Property node for boolean values true and false
struct True : AnyBoolean { NODE_FUNCTIONS };
struct False : AnyBoolean { NODE_FUNCTIONS };

//////////////////////////// Declarations /////////////////////

// An Identifier is a name given to a user-defined entity within 
// the program (variable/object/function, user-defined type or
// label). In the inferno tree, these are fully scope resolved
// and are maintained as unique nodes so that the declaration
// and all usages all point to the same node, this preserving
// identity via topology. We store a string, but it isn't strictly 
// needed and there's no need to uniquify it (it's really just 
// a hint for users examining the output).
// TODO make sure renderer really is uniquifying where needed
struct Identifier : virtual Node { NODE_FUNCTIONS };

// Stores a name found in the program, eg identifier names.
// This is for unquoted strings, as opposed to String. Strictly,
// Inferno doesn't need to keep this data, but it helps
// to make renders and graphs clearer. This could use
// something like stack<string> if it makes manufacturing 
// names for new objects easier.
struct Named : virtual Property
{ 
    string name;
    NODE_FUNCTIONS 
};

// Identifier for an instance (variable or object or function)
// that can be any instance.
struct AnyInstanceIdentifier : Identifier,
                               Expression { NODE_FUNCTIONS };
                               
// Identifier for a specific instance that has been declared
// somewhere.                               
struct InstanceIdentifier : AnyInstanceIdentifier,
                            Named { NODE_FUNCTIONS };
                            

// Identifier for a user defined type that can be any type.
struct AnyTypeIdentifier : Identifier,
                           Type { NODE_FUNCTIONS };
                           
// Identifier for a specific user defined type that has been 
// declared somewhere.
struct TypeIdentifier : AnyTypeIdentifier,
                        Named { NODE_FUNCTIONS };
                      

// Identifier for a label that can be any label. 
struct AnyLabelIdentifier : Identifier,
                            Expression { NODE_FUNCTIONS };

// Identifier for a specific label that has been declared somewhere.
struct LabelIdentifier : AnyLabelIdentifier,
                         Named { NODE_FUNCTIONS };

// Property for whether a member function has been declared as virtual.
// We will add pure as an option here too. 
struct AnyVirtual : Property { NODE_FUNCTIONS };
struct Virtual : AnyVirtual 
{
    NODE_FUNCTIONS
    // TODO pure when supported by clang
};
struct NonVirtual : AnyVirtual { NODE_FUNCTIONS };

// Property for a storage class which can apply to any instance (variable,
// object or function) and indicates physical locaiton, allocation strategy 
// and lifecycle model. Presently we allow static, member (=non-static member)
// and auto (= non-static local). Member must also indicate virtual-ness.
// Note that top-level static -> Static and Private. 
// Top-level extern -> Static and Public.
struct StorageClass : Property { NODE_FUNCTIONS };
struct Static : StorageClass { NODE_FUNCTIONS };
struct Member : StorageClass // non-static
{
    NODE_FUNCTIONS
    SharedPtr<AnyVirtual> virt;
};
struct Auto : StorageClass { NODE_FUNCTIONS };

// Property that indicates whether some variable or object is constant.
struct AnyConst : Property { NODE_FUNCTIONS };
struct Const : AnyConst { NODE_FUNCTIONS };
struct NonConst : AnyConst { NODE_FUNCTIONS }; 
// TODO add mutable when supported by clang

// Intermediate for anything that consumes space and/or has state. Slightly 
// wooly concept but it gathers the properties that cover allocation of space 
// and read-only/read-write rules.
struct Physical : virtual Node
{
    NODE_FUNCTIONS
    SharedPtr<AnyConst> constant; 
    SharedPtr<StorageClass> storage;
};

// Node represents a variable/object or a function. In case of function, type is a 
// type under Subroutine and initialiser is a Statement (or Uninitialised for a function
// declaration). For a variable/object, type is basically anything else, and if there is
// an initialiser, it is an Expression. We allow init here for variaous reasons including
// - it can be hard to know where to put stand-alone init for statics
// - C++ constructors tie init to declaration
// - Fits in with single-static-assignment style
// The instance node is a declaration and goes into a declaration scope. It points
// to an InstanceIdentifier, and all usages of the instance actually point to the
// InstanceIdentifier.
struct Instance : Declaration,
                  Physical
{
    NODE_FUNCTIONS
    SharedPtr<Type> type;
    SharedPtr<AnyInstanceIdentifier> identifier;
    SharedPtr<Initialiser> initialiser; // NULL if uninitialised
};

// Node for a base class within a class declaration, specifies another class from 
// which to inherit
struct InheritanceRecord;
struct Base : Declaration
{
    NODE_FUNCTIONS
    SharedPtr<TypeIdentifier> record; // must refer to InheritanceRecord
    // TODO virtual inheritance, treat seperately from virtual members
    // since different thing.
};              

//////////////////////////// Anonymous Types ////////////////////////////

// Subroutine like in Basic, no params or return.
// Types under Subroutine refer to a function's interface as seen by 
// caller and as used in eg function pointers (which is simply Pointer to
// the function type). To actually have a function, with a body, you need
// an Instance with type filled in to something derived from Subroutine.
struct Subroutine : Type 
{
    NODE_FUNCTIONS
};

// A procedure like in pascal etc, params but no return value. Parameters are generated as 
// a sequence of automatic variable/object declarations (ie Instances).
struct Procedure : Subroutine
{
    NODE_FUNCTIONS
    Sequence<Instance> parameters;
};

// A function like in C, Pascal; params and a single return value of the specified type.
struct Function : Procedure
{
    NODE_FUNCTIONS
    SharedPtr<Type> return_type;
};

// A C++ constructor. The init list is just zero or more calls to constructors 
// in the body
struct Constructor : Procedure { NODE_FUNCTIONS };

// A C++ destructor
struct Destructor : Subroutine { NODE_FUNCTIONS };

// This is the type of an array that contains the specified number of elements
// of the specified type.
struct Array : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> element;
    SharedPtr<Initialiser> size; // Uninitialised if not given eg []
};

// A pointer to objects of some type, as specified.
struct Pointer : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> destination;
};

// A C++ reference to objects of some type, as specified.
struct Reference : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> destination;
};

// The pseudo-type void, disallowed in some circumstances as per C.
struct Void : Type { NODE_FUNCTIONS };

// Boolean type. We support bool seperately from 1-bit ints, at least for now.
// (note that (bool)2==true but (int:1)2==0)
struct Bool : Type { NODE_FUNCTIONS };

// Intermediate for any type that represents a number that you can eg add and 
// subtract. 
struct Numeric : Type { NODE_FUNCTIONS };

// Type represents an integral (singed or unsigned) type. The total number of
// bits (including sign) is given
struct Integral : Numeric
{
    NODE_FUNCTIONS
    SharedPtr<AnyInteger> width;  // Bits, not bytes
};

// Type of a signed integer number (2's complement).
struct Signed : Integral { NODE_FUNCTIONS };

// Type of an unsigned integer number.
struct Unsigned : Integral { NODE_FUNCTIONS };

// Property for the details of floating point behaviour
// implying representation size and implementation.
struct AnyFloatSemantics : Property { NODE_FUNCTIONS };
struct FloatSemantics : AnyFloatSemantics
{
    NODE_FUNCTIONS
    const llvm::fltSemantics *value;
};    

// Type of a floating point number.
struct Floating : Numeric 
{ 
    NODE_FUNCTIONS
    SharedPtr<AnyFloatSemantics> semantics;
}; 

//////////////////////////// User-defined Types ////////////////////////////

// Intermediate declaration of a user defined type of any kind (struct, typedef etc).
// The user type node is a declaration and goes into a declaration scope. It points
// to a TypeIdentifier, and all usages of the type actually point to the
// TypeIdentifier.
struct UserType : Declaration 
{ 
    NODE_FUNCTIONS
    SharedPtr<AnyTypeIdentifier> identifier;
};

// Represents a typedef. Typedef is to the specified type.
struct Typedef : UserType
{
    NODE_FUNCTIONS
    SharedPtr<Type> type;
}; 

// Property nodes for indicating whether a record has been forward declared
// (incomplete) or fully declared (complete). TODO we should now be able to 
// drop incomplete records and always use the complete version, so don't
// need this.
struct AnyComplete : Property { NODE_FUNCTIONS };
struct Complete : AnyComplete { NODE_FUNCTIONS };
struct Incomplete : AnyComplete { NODE_FUNCTIONS };

// Intermediate for declaration of a record. record is generic for struct, class, union or
// enum. We list the memebrs here as declaraions (which will be member 
// or static) can can be variables/objects in all cases and additionally
// function instances in struct/class. Record completeness of declaration too.
struct Record : UserType
{
    NODE_FUNCTIONS
    Sequence<Declaration> members;
    
    // TODO get rid; always refer to complete version
    SharedPtr<AnyComplete> complete; 
};

// A union, as per Record.
struct Union : Record { NODE_FUNCTIONS };

// An Enum, as per record. We regard enumerations as static const
// variables, initialised as per the given value.
struct Enum : Record { NODE_FUNCTIONS };

// A record that can inherit from other records and be inherited from. 
// We add in a list of base class declarations.
struct InheritanceRecord : Record
{
    NODE_FUNCTIONS
    Sequence<Base> bases;
};

// Struct and class as per InheritanceRecord
struct Struct : InheritanceRecord { NODE_FUNCTIONS };
struct Class : InheritanceRecord { NODE_FUNCTIONS };

//////////////////////////// Expressions ////////////////////////////

// Declaration of a label for switch, goto etc.
// The label node is a declaration and goes into a statement scope. It points
// to a LabelIdentifier, and all usages of the type actually point to the
// LabelIdentifier.
// TODO consider making this an object, STATIC and void * type
struct Label : Declaration
{
    NODE_FUNCTIONS
    SharedPtr<AnyLabelIdentifier> identifier;
}; 

// Aggregate is used to combine a number of values for initialising
// a record or an array. It should really derive from Initialiser
// since it can only be used as an initialiser and TypeOf would 
// have a hard time with it. But it's hard to do in parser due to
// the clang integration model in parser TODO do it anyway.
struct Aggregate : Expression
{
    NODE_FUNCTIONS
    Sequence<Expression> operands; 
};

// Intermediate for an operator with operands and an indication
// of whether it's an assignment operator. TODO maybe fix the number
// of operands for binop and unop categories instead of Sequence.
struct Operator : Expression
{
    NODE_FUNCTIONS
    Sequence<Expression> operands; 
};

// Intermendiate categories of operators. TODO not sure if this
// is the right categorisation scheme.
struct Bitwise : Operator { NODE_FUNCTIONS };
struct Logical : Operator { NODE_FUNCTIONS };
struct Arithmetic : Operator { NODE_FUNCTIONS };
struct Shift : Operator { NODE_FUNCTIONS };
struct Comparison : Operator { NODE_FUNCTIONS };
struct AssignmentOperator : Operator { NODE_FUNCTIONS };

// Use an include file to generate nodes for all the actual operatos based on
// contents of operator_db.inc
#define PREFIX(TOK, TEXT, NODE, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#define POSTFIX(TOK, TEXT, NODE, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#define INFIX(TOK, TEXT, NODE, BASE) struct NODE : BASE { NODE_FUNCTIONS };
#include "operator_db.inc"

// Operator that operates on data types as parameters. Where either is allowed
// we prefer the type one, since it's more concise in the tree.
struct TypeOperator : Expression
{
    NODE_FUNCTIONS
    Sequence<Type> operands; 
};

// sizeof() a type
struct SizeOf : TypeOperator { NODE_FUNCTIONS };

// alignof() a type
struct AlignOf : TypeOperator { NODE_FUNCTIONS };

// The conditional ?: operator as in condition ? if_true : if_false
struct ConditionalOperator : Expression 
{
    NODE_FUNCTIONS
    SharedPtr<Expression> condition;
    SharedPtr<Expression> if_true;
    SharedPtr<Expression> if_false;
};

// A funciton call to specified function passing in specified arguments
struct Call : Expression 
{
    NODE_FUNCTIONS
    SharedPtr<Expression> function;
    Sequence<Expression> arguments;
};

// Property indicating whether a new/delete is global ie has :: in
// front of it. Not actually sure what that means TODO find out
struct AnyGlobalNew : Property { NODE_FUNCTIONS };
struct GlobalNew : AnyGlobalNew { NODE_FUNCTIONS }; // ::new/::delete was used
struct NonGlobalNew : AnyGlobalNew { NODE_FUNCTIONS }; // new/delete, no ::

// Property indicating whether a delete should delete an array
struct AnyArrayNew : Property { NODE_FUNCTIONS };
struct ArrayNew : AnyArrayNew { NODE_FUNCTIONS }; // delete[]
struct NonArrayNew : AnyArrayNew { NODE_FUNCTIONS }; 

// Node for the C++ new operator, gives all the syntactical elements
// required for allocation and initialisation
struct New : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Type> type; 
    Sequence<Expression> placement_arguments;
    Sequence<Expression> constructor_arguments;
    SharedPtr<AnyGlobalNew> global;
};

// Node for C++ delete operator
struct Delete : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Expression> pointer;
    SharedPtr<AnyArrayNew> array;
    SharedPtr<AnyGlobalNew> global;
};

// Node for C++ this
struct This : Expression { NODE_FUNCTIONS };

// Node for indexing into an array as in base[index]
struct Subscript : Expression 
{
    NODE_FUNCTIONS
    SharedPtr<Expression> base;
    SharedPtr<Expression> index;
};

// Node for accessing an element in a record as in base.member
// Note that the parser breaks down a->b into (*a).b which may
// be detected using a search pattern if desired.
struct Lookup : Expression  
{
    NODE_FUNCTIONS
    SharedPtr<Expression> base; 
    SharedPtr<AnyInstanceIdentifier> member;    
};

// Node for a c-style cast. C++ casts are not in here yet
// and C casts will be harmonised into whatever system I use for that.
struct Cast : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Expression> operand;
    SharedPtr<Type> type;        
};

//////////////////////////// Statements ////////////////////////////

// Bunch of statements inside {} or begin/end
struct Compound : Statement
{
    NODE_FUNCTIONS
    Sequence<Statement> statements;
};                   

// The return statement of a function TODO not sure about return without 
// a paremeter, might generate NULL ptr which would be invalid.
struct Return : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Expression> return_value;
};

// A label as it appears in code, followed by a :
struct LabelTarget : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Label> label; // TODO these should be function scope
};

// A goto statement, inferno tree supports goto-a-variable because
// it is expected to be useful during sequential lowering (state-out)
struct Goto : Statement
{
    NODE_FUNCTIONS
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
    SharedPtr<Expression> destination;
};

// If statement
struct If : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Expression> condition;
    SharedPtr<Statement> body;
    SharedPtr<Statement> else_body; // can be Nop if no else clause
};

// Intermediate for any loop, commonising the body
struct Loop : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Statement> body;
};

// While loop
struct While : Loop
{
    NODE_FUNCTIONS
    SharedPtr<Expression> condition;
};

// Do loop (first iteration always runs)
struct Do : Loop // a do..while() construct 
{
    NODE_FUNCTIONS
    SharedPtr<Expression> condition;
};

// For loop. 
struct For : Loop
{
    NODE_FUNCTIONS
    SharedPtr<Statement> initialisation; // Nop if absent
    SharedPtr<Expression> condition;     // True if absent
    SharedPtr<Statement> increment;      // Nop if absent
};

// Switch statement. Body is just a statement scope - case labels
// and breaks are dropped into the sequence at the corresponding 
// positions. This caters for fall-throughs etc. Really just a 
// Compound with a goto-a-variable at the top with some mapping.
struct Switch : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Expression> condition;
    SharedPtr<Statement> body;
};

// Intermediate for labels in a switch statement.
struct SwitchTarget : Statement { NODE_FUNCTIONS };

// Case label, supporting range extension in case useful
// for optimisation
struct Case : SwitchTarget 
{
    NODE_FUNCTIONS
    // support gcc extension of case x..y:
    // in other cases, value_lo==value_hi
    SharedPtr<Expression> value_lo; // inclusive
    SharedPtr<Expression> value_hi; // inclusive
};

// Default label in a switch statement
struct Default : SwitchTarget { NODE_FUNCTIONS };

// Continue (to innermost Loop)
struct Continue : Statement { NODE_FUNCTIONS };

// Break (from innermost Switch or Loop)
struct Break : Statement { NODE_FUNCTIONS };

// Do nuffink
struct Nop : Statement { NODE_FUNCTIONS };

#endif
