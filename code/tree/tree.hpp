#ifndef TREE_HPP
#define TREE_HPP

#include "common/refcount.hpp"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include <string>
#include <deque>
#include "generics.hpp"

//TODO all these in a name space perhaps?

//////////////////////////// Node Model ////////////////////////////

// Base class for all tree nodes and nodes in search/replace 
// patterns etc. Convention is to use "struct" for derived
// node classes so that everything is public (inferno tree nodes
// are more like records in a database, they have only minimal 
// functionality). Also, all derived structs should contain the
// NODE_FUNCTIONS macro which expands to a few virtual functions
// required for common ("bounced") functionality. Where multiple
// inheritance diamonds arise, Node should be derived virtually
// (we always want the set-restricting model of inheritance in
// the inferno tree node hierarchy).
struct Node : NodeBases
{            
    NODE_FUNCTIONS
   
    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds 
    // without making Node ambiguous  
};

//////////////////////////// Underlying Node Types ////////////////////////////

// Property is the base class for property nodes. Each kind of property has an
// intermediate which can represent any value of the property. Enum-like and
// bool-like are implemented by choosing one of a choice of empty nodes derived
// from the intermediate. Other properties that cannot be represented this way
// have a Specific<Foo> node that actually contains the datatype (eg int, string
// etc). The intermediates should be the target of SharedPtrs and may be used in
// search patterns. The actual tree nodes for a program should always be the leaf
// node type.
struct Property : virtual Node { NODE_FUNCTIONS };

// This intermediate is used for an initial value for for a variable/object in
// which case it will be an Expression, or for the implementation of a function
// in which case it will be a Statement. For an uninitialised variable/object
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

// A declaration specifies the creation of a type or an object from a type. 
// Declaration can appear where statements can and also inside structs etc
// and at top level.
struct Declaration : virtual Node { NODE_FUNCTIONS };

// A scope is any space in a program where declarations may appear. Declarations
// in the collection are associated with the scope node but unordered. Scopes
// are used for name resolution during parse.
struct Scope : virtual Node
{
    NODE_FUNCTIONS
    Collection<Declaration> members;
};

// The top level of a program is considered a collection of declarations.
// main() would typically be a function instance somewhere in this collection.
struct Program : Scope { NODE_FUNCTIONS };

//////////////////////////// Literals ///////////////////////////////

// Means a property that can be used as a literal in a program, so
// that we do not need to duplicate literals and properties.
struct Literal : Expression,
                 Property
{
    NODE_FUNCTIONS
};

// Intermediate property node that represents a string of any value.
struct String : Literal { NODE_FUNCTIONS };

// A string with a particular value as specified. Value must be filled
// in.
struct SpecificString : String
{
    NODE_FUNCTIONS
    string value;
};

// Intermediate property node that represents a number (anything you
// can do +, - etc on) of any value.
struct Number : Literal { NODE_FUNCTIONS };

#define INTEGER_DEFAULT_WIDTH 32

// Intermediate property node that represents an integer number of any
// value (signed or unsigned).
struct Integer : Number { NODE_FUNCTIONS };

// Property node for an integer number. We use LLVM's class for this, 
// so that we can deal with any size of number (so this can be used for
// large bit vectors). The LLVM object also stores the signedness. The
// value must always be filled in.
struct SpecificInteger : Integer
{
    NODE_FUNCTIONS
    SpecificInteger( int i ) : value(INTEGER_DEFAULT_WIDTH) { value = i; }
    SpecificInteger() : value(INTEGER_DEFAULT_WIDTH) { value = 0; }
    llvm::APSInt value; // APSint can be signed or unsigned
};

// Intermediate property node that represents a floating point number of any
// value.
struct Float : Number { NODE_FUNCTIONS };

// Property node for an floating point number. We use LLVM's class for this, 
// so that we can deal with any representation convention. The value must 
// always be filled in. To determine the type, use llvm::getSemantics()
struct SpecificFloat : Float
{
    NODE_FUNCTIONS
    SpecificFloat() : value((float)0) {};
    SpecificFloat( llvm::APFloat v ) : value(v) {};
    llvm::APFloat value; 
};

// Intermediate property node that represents any boolean value.
// Note: Bool here is considered a noun, and in general Property/Literal
// intermediates are named using nouns. C.f. the Type node Boolean
struct Bool : Literal { NODE_FUNCTIONS };

// Property node for boolean values true and false
struct True : Bool { NODE_FUNCTIONS };
struct False : Bool { NODE_FUNCTIONS };

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
struct Identifier : virtual Property { NODE_FUNCTIONS };

// Stores a name found in the program, eg identifier names.
// This is for unquoted strings, as opposed to String. Strictly,
// Inferno doesn't need to keep this data, but it helps
// to make renders and graphs clearer. This could use
// something like stack<string> if it makes manufacturing 
// names for new objects easier.
struct SpecificIdentifier : virtual Property
{ 
	SpecificIdentifier() {}
	SpecificIdentifier( string s ) : name(s) {}
	//virtual bool IsLocalMatch( const Matcher &candidate ) const
    string name;
    NODE_FUNCTIONS 
};

// Identifier for an instance (variable or object or function)
// that can be any instance.
struct InstanceIdentifier : Identifier,
                            Expression { NODE_FUNCTIONS };
                               
// Identifier for a specific instance that has been declared
// somewhere.                               
struct SpecificInstanceIdentifier : InstanceIdentifier,
                                    SpecificIdentifier
{
	SpecificInstanceIdentifier() {}
	SpecificInstanceIdentifier( string s ) : SpecificIdentifier(s) {}
    NODE_FUNCTIONS
};
                            

// Identifier for a user defined type that can be any type.
struct TypeIdentifier : Identifier,
                        Type { NODE_FUNCTIONS };
                           
// Identifier for a specific user defined type that has been 
// declared somewhere.
struct SpecificTypeIdentifier : TypeIdentifier,
                                SpecificIdentifier
{
	SpecificTypeIdentifier() {}
	SpecificTypeIdentifier( string s ) : SpecificIdentifier(s) {}
    NODE_FUNCTIONS
};

// Identifier for a label that can be any label. 
struct LabelIdentifier : Identifier,
                         Expression { NODE_FUNCTIONS };

// Identifier for a specific label that has been declared somewhere.
struct SpecificLabelIdentifier : LabelIdentifier,
                                 SpecificIdentifier
{
	SpecificLabelIdentifier() {}
	SpecificLabelIdentifier( string s ) : SpecificIdentifier(s) {}
    NODE_FUNCTIONS
};

// General note about identifiers: in a valid program tree, there should
// be *one* Declaration node that points to the identifier and serves to 
// declare it. There should be 0 or more "users" that point to the
// identifier. 

// Property for whether a member function has been declared as virtual.
// We will add pure as an option here too. 
struct Virtuality : Property { NODE_FUNCTIONS };
struct Virtual : Virtuality
{
    NODE_FUNCTIONS
    // TODO pure when supported by clang
};
struct NonVirtual : Virtuality { NODE_FUNCTIONS };

// Property for C++ access specifiers public, protected and private. AccessSpec
// represents any access spec, the subsequent empty nodes specify particular
// access specs. Inferno uses access specs for all declarations. Declarations
// in classes are as specified, function parameters and external declarations
// are public, all others private. It is anticipated that the access spec will
// control generated high-level interfaces. Note that we only specify access
// for physical things like instances. Abstract stuff like typedefs are always
// considered public.
struct AccessSpec : Property { NODE_FUNCTIONS };
struct Public : AccessSpec { NODE_FUNCTIONS };
struct Private : AccessSpec { NODE_FUNCTIONS };
struct Protected : AccessSpec { NODE_FUNCTIONS };

// Intermediate for anything that consumes space and/or has state. Slightly 
// wooly concept.
struct Physical : Declaration
{
    NODE_FUNCTIONS
    SharedPtr<AccessSpec> access;
};

// Property for a storage class which can apply to any instance (variable,
// object or function) and indicates physical location, allocation strategy 
// and life-cycle model. Presently we allow static, member (=non-static member)
// and auto (= non-static local). Member must also indicate virtual-ness.
// Note that top-level static -> Static and Private. 
// Top-level extern -> Static and Public.
struct StorageClass : Property { NODE_FUNCTIONS };
struct Static : StorageClass { NODE_FUNCTIONS };
struct Member : StorageClass
{
    NODE_FUNCTIONS
    SharedPtr<Virtuality> virt;
};
struct Auto : StorageClass { NODE_FUNCTIONS };
struct Temp : StorageClass { NODE_FUNCTIONS }; // like Auto but cannot recurse

// Property that indicates whether some variable or object is constancy.
struct Constancy : Property { NODE_FUNCTIONS };
struct Const : Constancy { NODE_FUNCTIONS };
struct NonConst : Constancy { NODE_FUNCTIONS }; 
// TODO add mutable when supported by clang

// Node represents a variable/object or a function. In case of function, type is a 
// type under Subroutine and initialiser is a Statement (or Uninitialised for a function
// declaration). For a variable/object, type is basically anything else, and if there is
// an initialiser, it is an Expression. We allow init here for various reasons including
// - it can be hard to know where to put stand-alone init for statics
// - C++ constructors tie init to declaration
// - Fits in with single-static-assignment style
// The instance node can go into a Declaration Collection or a Statement Sequence.
// THe latter case is used where initialisaiton/construction demands ordering. It points
// to an InstanceIdentifier, and all usages of the instance actually point to the
// InstanceIdentifier.
struct Instance : Physical,
                  Statement
{
    NODE_FUNCTIONS
    SharedPtr<StorageClass> storage; // TODO move to Physical so Base has one
    SharedPtr<Constancy> constancy; 
    SharedPtr<Type> type;
    SharedPtr<InstanceIdentifier> identifier;
    SharedPtr<Initialiser> initialiser; // NULL if uninitialised
};

// Node for a base class within a class declaration, specifies another class from 
// which to inherit
struct Base : Physical 
{
    NODE_FUNCTIONS
    SharedPtr<TypeIdentifier> record; // must refer to InheritanceRecord
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
struct Procedure : Subroutine,
                   Scope // For the parameters
{
    NODE_FUNCTIONS
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

// Intermediate for indirect access to some type as specified
// (basically, pointers and references)
struct Indirection : Type
{
    NODE_FUNCTIONS
    SharedPtr<Type> destination;	
};

// A C/C++ pointer
struct Pointer : Indirection { NODE_FUNCTIONS };

// A C++ reference
struct Reference : Indirection { NODE_FUNCTIONS };

// The pseudo-type void, disallowed in some circumstances as per C.
struct Void : Type { NODE_FUNCTIONS };

// Boolean type. We support bool separately from 1-bit ints, at least for now.
// (note that (bool)2==true but (int:1)2==0)
// Note: Boolean here is considered an adjective, and in general Type
// nodes are named using adjectives. C.f. the Property/Literal intermediate Bool
struct Boolean : Type { NODE_FUNCTIONS };

// Intermediate for any type that represents a number that you can eg add and 
// subtract. 
struct Numeric : Type { NODE_FUNCTIONS };

// Type represents an integral (singed or unsigned) type. The total number of
// bits (including sign) is given
struct Integral : Numeric
{
    NODE_FUNCTIONS
    SharedPtr<Integer> width;  // Bits, not bytes
};

// Type of a signed integer number (2's complement).
struct Signed : Integral { NODE_FUNCTIONS };

// Type of an unsigned integer number.
struct Unsigned : Integral { NODE_FUNCTIONS };

// Property for the details of floating point behaviour
// implying representation size and implementation.
struct FloatSemantics : Property { NODE_FUNCTIONS };
struct SpecificFloatSemantics : FloatSemantics
{
    NODE_FUNCTIONS
    const llvm::fltSemantics *value;
};    

// Type of a floating point number.
struct Floating : Numeric 
{ 
    NODE_FUNCTIONS
    SharedPtr<FloatSemantics> semantics;
}; 

//////////////////////////// User-defined Types ////////////////////////////

// Intermediate declaration of a user defined type of any kind (struct, typedef etc).
// The user type node is a declaration and goes into a declaration scope. It points
// to a TypeIdentifier, and all usages of the type actually point to the
// TypeIdentifier.
struct UserType : Declaration 
{ 
    NODE_FUNCTIONS
    SharedPtr<TypeIdentifier> identifier;
};

// Represents a typedef. Typedef is to the specified type.
struct Typedef : UserType
{
    NODE_FUNCTIONS
    SharedPtr<Type> type;
}; 

// Intermediate for declaration of a record. record is generic for struct, class, union or
// enum. We list the members here as declarations (which will be member
// or static) can can be variables/objects in all cases and additionally
// function instances in struct/class.
struct Record : UserType,
                Scope // Member declarations go in here
{
    NODE_FUNCTIONS
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
    Collection<Base> bases; // TODO just chuck them into Record::members? TODO Collection?
};

// Struct and class as per InheritanceRecord
struct Struct : InheritanceRecord { NODE_FUNCTIONS };
struct Class : InheritanceRecord { NODE_FUNCTIONS };

//////////////////////////// Expressions ////////////////////////////

// Declaration of a label for switch, goto etc.
// This node represents a label such as mylabel: 
// It serves to declare the label; the identifier should be 
// used for references.
struct Label : Declaration, // TODO be a Statement TODO commonize with Case and Default TODO move if not an Expression
               Statement
{
    NODE_FUNCTIONS
    SharedPtr<LabelIdentifier> identifier;
}; 

// Intermediate for an operator with operands. TODO maybe fix the number
// of operands for binop and unop categories.
struct Operator : Expression
{
	NODE_FUNCTIONS
};

// Associates an instance with an expression. Basically a
// key-value pair of identifier and value. Use in Maps.
struct MapOperand : virtual Node
{
	NODE_FUNCTIONS
	SharedPtr<InstanceIdentifier> identifier;
	SharedPtr<Expression> value;
};

// Operator that maps a multiplicity of instances to expressions
// via their identifiers.
struct MapOperator : Operator
{
	NODE_FUNCTIONS
	Collection<MapOperand> operands;
};

struct NonCommutativeOperator : Operator
{
	NODE_FUNCTIONS
	Sequence<Expression> operands;
};

struct CommutativeOperator : Operator
{
	NODE_FUNCTIONS
	Collection<Expression> operands;
};

// Intermediate categories of operators. We categorise based on
// topology, and commutative is considered topologically
// distinct from non-commutative.
struct Unop : NonCommutativeOperator { NODE_FUNCTIONS };
struct Binop : NonCommutativeOperator { NODE_FUNCTIONS };
struct Ternop : NonCommutativeOperator { NODE_FUNCTIONS };
struct CommutativeBinop : CommutativeOperator { NODE_FUNCTIONS };
struct AssignmentOperator : NonCommutativeOperator { NODE_FUNCTIONS };

// Use an include file to generate nodes for all the actual operators based on
// contents of operator_db.inc
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS };
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS };
#define INFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS };
#include "operator_db.inc"

// Operator that operates on data types as parameters. Where either is allowed
// we prefer the type one, since it's more concise.
struct TypeOperator : Operator
{
    NODE_FUNCTIONS
    SharedPtr<Type> operand;
};

// sizeof() a type
struct SizeOf : TypeOperator { NODE_FUNCTIONS }; // TODO provide normal Unop versions of this since using TypeOf is causing multiple pointers to the same type node

// alignof() a type
struct AlignOf : TypeOperator { NODE_FUNCTIONS };

// The conditional ?: operator as in operands[0] ? operands[1] : operands[2]
struct ConditionalOperator : Ternop
{
    NODE_FUNCTIONS
};

// A function call to specified function passing in specified arguments
// Function is an expression to allow eg function pointer dereference. Normal
// calls have callee -> some InstanceIdentifier for a Subroutine Instance.
// Arguments passed via MapOperator - mapped to the parameters in the callee
// type (if it's a Procedure).
struct Call : MapOperator
{
    NODE_FUNCTIONS
    SharedPtr<Expression> callee;
};

// Property indicating whether a new/delete is global ie has :: in
// front of it. This differentiates when placement args are given as follows:
// Global: must be one placement arg, it is address to construct at
// NonGlobal: all placement args go to a corresponding operator new which returns address to construct at
// TODO bring these in line with Call etc
struct Globality : Property { NODE_FUNCTIONS };
struct Global : Globality { NODE_FUNCTIONS }; // ::new/::delete was used
struct NonGlobal : Globality { NODE_FUNCTIONS }; // new/delete, no ::

// Property indicating whether a delete should delete an array.
// Apologies for the tenuous grammar.
struct DeleteArrayness : Property { NODE_FUNCTIONS };
struct DeleteArray : DeleteArrayness { NODE_FUNCTIONS }; // delete[]
struct DeleteNonArray : DeleteArrayness { NODE_FUNCTIONS }; // delete, no []

// Node for the C++ new operator, gives all the syntactical elements
// required for allocation and initialisation
struct New : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Type> type; 
    Sequence<Expression> placement_arguments;
    Sequence<Expression> constructor_arguments;
    SharedPtr<Globality> global;
};

// Node for C++ delete operator
struct Delete : Expression // TODO Statement surely?
{
    NODE_FUNCTIONS
    SharedPtr<Expression> pointer;
    SharedPtr<DeleteArrayness> array;
    SharedPtr<Globality> global;
};

// Node for C++ this pointer
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
    SharedPtr<InstanceIdentifier> member;
};

// Node for a c-style cast. C++ casts are not in here yet
// and C casts will be harmonised into whatever scheme I use for that.
struct Cast : Expression
{
    NODE_FUNCTIONS
    SharedPtr<Expression> operand;
    SharedPtr<Type> type;        
};

// Initialiser for an array just lists the elements in order
struct ArrayLiteral : Operator
{
    NODE_FUNCTIONS
    Sequence<Expression> elements;
};

// Initialiser for a record uses a map to associate elements with
// corresponding record members. We also give the record type explicitly.
struct RecordLiteral : MapOperator
{
	NODE_FUNCTIONS
	SharedPtr<TypeIdentifier> type;
};

//////////////////////////// Statements ////////////////////////////

// Bunch of statements inside {} or begin/end. Note that local declarations
// can go in the members of the Scope or in the statements (since Declaration
// derives from Statement)
struct Compound : Statement,
                  Scope  // Local declarations go in here (preferably)
{
    NODE_FUNCTIONS
    Sequence<Statement> statements; // Can contain local declarations and code
};                   

// The return statement of a function; return_value is an Expression
// giving the return value or Uninitialised if none is present. 
struct Return : Statement
{
    NODE_FUNCTIONS
    SharedPtr<Initialiser> return_value;
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
// Compound with a goto-a-variable at the top and some mapping.
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
