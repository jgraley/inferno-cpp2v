#ifndef CPPTREE_HPP
#define CPPTREE_HPP

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include <string>
#include "node/specialise_oostd.hpp"
#include "clang/Parse/DeclSpec.h"
#include "tree/type_db.hpp"

/// CPPTree namespace contains node definitions that represent elements of the C++ language
namespace CPPTree {

//////////////////////////// Underlying Node Types ////////////////////////////

/// Property is the base class for property nodes. 
/** Each kind of property has an
 intermediate which can represent any value of the property. Enum-like and
 bool-like are implemented by choosing one of a choice of empty nodes derived
 from the intermediate. Other properties that cannot be represented this way
 have a Specific<Foo> node that actually contains the datatype (eg int, string
 etc). The intermediates should be the target of SharedPtrs and may be used in
 search patterns. The actual tree nodes for a program should always be the leaf
 node type. */
struct Property : virtual Node { NODE_FUNCTIONS };

/// Variable initialiser or function body
/** This intermediate is used for an initial value for for a variable/object in
 which case it will be an Expression, or for the implementation of a Callable
 in which case it will be a Compound. For an uninitialised variable/object
 or a function declaration, it will be Uninitialised. */
struct Initialiser : virtual Node { NODE_FUNCTIONS };

/// an uninitialised Instance.
struct Uninitialised : Initialiser { NODE_FUNCTIONS_FINAL }; 

/// Represents a statement as found inside a function body. 
/** Basically anything 
 that ends with a ; inside a function body, as well as labels (which we consider as 
 statements in their own right). */
struct Statement : virtual Node { NODE_FUNCTIONS };

/// An expression that computes a result value. 
/** Can be used anywhere a statement can, per C syntax rules. */
struct Expression : virtual Statement,
                    Initialiser { NODE_FUNCTIONS };

/// Any abstract data type
/** Any abstract data type including fundamentals, structs, function prototypes
 and user-named types. */
struct Type : virtual Node { NODE_FUNCTIONS };

/// A declaration specifies the creation of a UserType or an Instance. 
/** Declaration can appear where statements can and also inside structs etc
 and at top level. */
struct Declaration : virtual Node { NODE_FUNCTIONS };

/// A scope is any space in a program where declarations may appear. Declarations
/** in the collection are associated with the scope node but unordered. Scopes
 are used for name resolution during parse. */
struct Scope : virtual Node
{
    NODE_FUNCTIONS
    Collection<Declaration> members; /// The declarations in this scope
};

/// The top level of a program
/** The top level of a program is considered a collection of declarations.
 main() would typically be a function instance somewhere in this collection. */
struct Program : Scope { NODE_FUNCTIONS_FINAL };

/// Indicates that the node cannot be combinationalised
struct Uncombable : virtual Node { NODE_FUNCTIONS };

//////////////////////////// Literals ///////////////////////////////

/// A property that can also be used as a literal in a program
/** There are also used as properties, so that we do not need to 
 duplicate literals and properties. */
struct Literal : Expression,
                 Property
{
    NODE_FUNCTIONS
};

/// Intermediate property node that represents a string of any value.
struct String : Literal { NODE_FUNCTIONS };

/// A string with a specific value 
/** Value must be filled in. 
 TODO could be a problem with memory management here or nearby. See
 comment in test harness in search_replace.cpp. */
struct SpecificString : String
{
	NODE_FUNCTIONS_FINAL
    SpecificString(); ///< default constructor, for making architypes 
    SpecificString( string s ); /// Construct with a given STL string
	virtual bool IsLocalMatch( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
	virtual string GetRender() const; /// Produce a string for debug
    virtual string GetTrace() const;
    
private:
    string value; ///< The string itself
};

/// Intermediate property node that represents a number of any value 
/** (anything you
 can do +, - etc on). */
struct Number : Literal { NODE_FUNCTIONS };

#define INTEGER_DEFAULT_WIDTH 32

/// Intermediate property node that represents an integer of any value and signedness. 
struct Integer : Number { NODE_FUNCTIONS };

/// Property node for a specific integer value. 
/** We use LLVM's class for this, 
 so that we can deal with any size of number (so this can be used for
 large bit vectors). The LLVM object also stores the signedness. The
 value must always be filled in. */
struct SpecificInteger : Integer, llvm::APSInt
{
	NODE_FUNCTIONS_FINAL
    SpecificInteger(); ///< default constructor, for making architypes 
    SpecificInteger( llvm::APSInt i ); ///< Construct with an LLVM-style integer
    SpecificInteger( int i ); ///< Construct with an ordinary int
	virtual bool IsLocalMatch( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
	virtual string GetRender() const; /// Produce a string for debug
    virtual string GetTrace() const;
};

/// Intermediate property node that represents a floating point number of any value. 
struct Float : Number { NODE_FUNCTIONS };

/// Property node for a specific floating point value.
/** We use LLVM's class for this, 
 so that we can deal with any representation convention. The value must 
 always be filled in. To determine the type, use llvm::getSemantics() */
struct SpecificFloat : Float, llvm::APFloat
{
	NODE_FUNCTIONS_FINAL
    SpecificFloat(); ///< default constructor, for making architypes 
    SpecificFloat( llvm::APFloat v ); ///< Construct with an LLVM-style float
	virtual bool IsLocalMatch( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
	virtual string GetRender() const; /// Produce a string for graphing
    virtual string GetTrace() const;
};

/// Intermediate property node that represents either boolean value.
/** Note: Bool here is considered a noun, and in general Property/Literal
 intermediates are named using nouns. C.f. the Type node Boolean */
struct Bool : Literal { NODE_FUNCTIONS };

/// Property node for boolean value true 
struct True : Bool
{
	NODE_FUNCTIONS_FINAL
	virtual string GetRender() const { return "true"; } ///< Produce a string for debug
};

/// Property node for boolean value false 
struct False : Bool
{
	NODE_FUNCTIONS_FINAL
	virtual string GetRender() const { return "false"; } ///< Produce a string for debug
};

//////////////////////////// Declarations /////////////////////

/// Property node for any identifier
/** An Identifier is a name given to a user-defined entity within 
 the program (variable/object/function, user-defined type or
 label). In the inferno tree, these are fully scope resolved
 and are maintained as unique nodes so that the declaration
 and all usages all point to the same node, this preserving

 identity via topology. We store a string, but it isn't strictly 
 needed and there's no need to uniquify it (it's really just 
 a hint for users examining the output). */
struct Identifier : virtual Property { NODE_FUNCTIONS };

/// Property for a specific identifier, linked to by a particular Declaration
/** This is for unquoted strings, as opposed to String. Strictly,
 Inferno doesn't need to keep this data, but it helps
 to make renders and graphs clearer. Inferno will uniquify
 the name when rendering code. */
struct SpecificIdentifier : virtual Property
{ 
    NODE_FUNCTIONS
	SpecificIdentifier(); ///< default constructor, for making architypes 
	SpecificIdentifier( string s ); ///< construct with a given name
    virtual shared_ptr<Cloner> Duplicate( shared_ptr<Cloner> p ); /// Overloaded duplication function for search&replace
	virtual bool IsLocalMatch( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
	virtual string GetRender() const; /// This is relied upon to just return the identifier name for rendering
    virtual string GetTrace() const;

private:
	string name;
};

/// Identifier for any Instance (variable or object or function)
struct InstanceIdentifier : Identifier,
                            Expression { NODE_FUNCTIONS };
                               
/// Identifier for a specific Instance, linked to by a particular Declaration                           
struct SpecificInstanceIdentifier : InstanceIdentifier,
                                    SpecificIdentifier
{
	SpecificInstanceIdentifier() {} ///< Default constructor
	SpecificInstanceIdentifier( string s ) : SpecificIdentifier(s) {} ///< make identifier with the given name
	NODE_FUNCTIONS_FINAL
};
                            

/// Identifier for any user defined type.
struct TypeIdentifier : Identifier,
                        Type { NODE_FUNCTIONS };
                           
/// Identifier for a specific user defined type, linked to by a particular Declaration.
struct SpecificTypeIdentifier : TypeIdentifier,
                                SpecificIdentifier
{
	SpecificTypeIdentifier() {} ///< Default constructor
	SpecificTypeIdentifier( string s ) : SpecificIdentifier(s) {} ///< make identifier with the given name
	NODE_FUNCTIONS_FINAL
};

// General note about identifiers: in a valid program tree, there should
// be *one* Declaration node that points to the identifier and serves to 
// declare it. There should be 0 or more "users" that point to the
// identifier. 

/// Property for a member function that may or may not be virtual.
struct Virtuality : Property { NODE_FUNCTIONS };

/// Property for a virtual member funciton
struct Virtual : Virtuality
{
	NODE_FUNCTIONS_FINAL
    // TODO pure when supported by clang
};
/// Property for a non-virtual member funciton
struct NonVirtual : Virtuality { NODE_FUNCTIONS_FINAL };

/// Property for any access spec
/** Property for C++ access specifiers public, protected and private. AccessSpec
 represents any access spec, the subsequent empty nodes specify particular
 access specs. Inferno uses access specs for all Instance. Declaration
 in Record are as specified, Function parameters and external Declaration
 are Public, all others Private. It is anticipated that the access spec will
 control generated high-level interfaces. Note that we only specify access
 for physical things like instances. Abstract stuff like TypeDef are always
 considered Public. */
struct AccessSpec : Property { NODE_FUNCTIONS };

/// Property for public access
struct Public : AccessSpec { NODE_FUNCTIONS_FINAL };

/// Property for private access
struct Private : AccessSpec { NODE_FUNCTIONS_FINAL };

/// Property for protected access
struct Protected : AccessSpec { NODE_FUNCTIONS_FINAL };

/// Property that indicates whether some Instance is constant.
struct Constancy : Property { NODE_FUNCTIONS };

/// Property indicating the Instance is constant
struct Const : Constancy { NODE_FUNCTIONS_FINAL };

/// Property indicating the Instance is not constant
struct NonConst : Constancy { NODE_FUNCTIONS_FINAL };
// TODO add mutable when supported by clang

/// Declaration of a variable, object or function
/** Instance represents a variable/object or a function. In case of function, type is a
 type under Callable and initialiser is a Statement (or Uninitialised for a function
 declaration). For a variable/object, type is basically anything else, and if there is
 an initialiser, it is an Expression. We allow init here for various reasons including
 - it can be hard to know where to put stand-alone init for statics
 - C++ constructors tie init to declaration
 - Fits in with single-static-assignment style
 The instance node can go into a Declaration Collection or a Statement Sequence.
 The latter case is used where initialisaiton/construction demands ordering. It points
 to an InstanceIdentifier, and all usages of the instance actually point to the
 InstanceIdentifier. */
struct Instance : Declaration,
                  Statement
{
    NODE_FUNCTIONS
    TreePtr<InstanceIdentifier> identifier; ///< acts as a handle for the instance, and holds its name only as a hint
    TreePtr<Type> type; ///< the Type of the instance, can be data or Callable type
    TreePtr<Initialiser> initialiser; ///< init value for data, body for Callable type
};

/// A variable or function with one instance across the entire program. 
/** This includes extern and
 static scope for globals, as well as static locals. If a Static variable is Const, then it may be
 regarded as a compile-time constant. A static constant function may be regarded as idempotent. */
struct Static : Instance
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Constancy> constancy; ///< is the instance constant (ie compile time value)?
};

/// A non-static member Instance (function or variable)
/** A variable or function with one instance for each object of the containing class, ie
 non-static members. Functions have a "this" pointer. Note that access and constancy
 are intended to control the generation of read/write lines for modules. This usage of
 Constancy differs from that in Static, so we do not try to introduce a common intermediate.
 Note that static members are Static, not Field */
struct Field : Instance
{
	NODE_FUNCTIONS_FINAL
	TreePtr<Virtuality> virt; ///< Is the field virtual?
    TreePtr<AccessSpec> access; ///< Is it accessible outside the current Scope?
    TreePtr<Constancy> constancy; ///< Is the field constant (ie only written by constructor)
};

/// Any variable local to a Compound-statement. Cannot be a function.
struct LocalVariable : Instance
{
	NODE_FUNCTIONS
};

/// A local variable with automatic allocation
/** A variable with one instance for each *invocation* of a function, ie
    non-static locals. Safe across recursion. Note that static locals
    are Static, not Automatic. */
struct Automatic : LocalVariable
{
	NODE_FUNCTIONS_FINAL
};

/// A local temp variable not preserved across function calls
/** A local variable with unspecified storage which may be used within a function but is not preserved
 across recursion or between calls (such a variable could safely be implemented as any of
 Static, Field or Automatic since it supports only those guarantees common to all). */
struct Temporary : LocalVariable
{
	NODE_FUNCTIONS_FINAL
};

/// Node for a base class within a class declaration, specifies another class from which to inherit
struct Base : Declaration
{
	NODE_FUNCTIONS_FINAL
    TreePtr<AccessSpec> access; ///< Can inherited members be accessed?
    TreePtr<TypeIdentifier> record; ///< what do we inherit? must refer to InheritanceRecord
};              

/// Identifier for a label that can be any label.
/** LabelIdentifier is an Expression as well as an Identifier so that
    it may be assigned to variables and used in ?: expressions as 
    supported in the gcc variable-label extenstion. Inferno tree
    does not require any representation of && or * - Labels
    and expressions may be mixed directly. */
struct LabelIdentifier : Identifier,
                         Expression { NODE_FUNCTIONS };

/// Identifier for a specific label that has been declared somewhere.
struct SpecificLabelIdentifier : LabelIdentifier,
                                 SpecificIdentifier
{
	SpecificLabelIdentifier() {} ///< Default constructor
	SpecificLabelIdentifier( string s ) : SpecificIdentifier(s) {} ///< construct with initial name
	NODE_FUNCTIONS_FINAL
};

/// Declaration of a label for switch, goto etc.
/** This node represents a label such as mylabel:
 It serves to declare the label; the identifier should be
 used for references. */
struct Label : Declaration, //TODO commonize with Case and Default
               Statement,
               Uncombable
{
	NODE_FUNCTIONS_FINAL
    TreePtr<LabelIdentifier> identifier; ///< a handle for the label to be referenced elewhere
};

//////////////////////////// Anonymous Types ////////////////////////////

/// Anything that can be called
/** Types under Callable refer to a function's interface as seen by 
 caller and as used in eg function pointers (which is simply Pointer to
 the function type). To actually have a function, with a body, you need
 an Instance with type filled in to something derived from Callable. */
struct Callable : Type
{
    NODE_FUNCTIONS
};

/// Anything that can be called and has parameters
/** Parameters are generated as a sequence of automatic variable/object 
 declarations (ie Instances) inside the Scope we derive from. */
struct CallableParams : Callable,
                        Scope // For the parameters
{
    NODE_FUNCTIONS
};

/// Anything that can be called and has parameters and return value
struct CallableParamsReturn : CallableParams
{
    NODE_FUNCTIONS
    TreePtr<Type> return_type; ///< The return type
};

/// Subroutine like in Basic, no params or return.
/** Note: Subroutine has *explicitly* no params or return, this may be relied upon
    in transformations that use Subroutine as a wildcard */
struct Subroutine : Callable 
{
    NODE_FUNCTIONS
};

/// A procedure like in pascal etc, params but no return value. 
/** Note: Procedure has *explicitly* no return, this may be relied upon
    in transformations that use Subroutine as a wildcard */
struct Procedure : CallableParams
{
    NODE_FUNCTIONS
};

/// A function like in C, Pascal; params and a single return value of the specified type.
struct Function : CallableParamsReturn
{
	NODE_FUNCTIONS_FINAL
};

/// A C++ constructor. The init list is just zero or more calls to constructors in the body
struct Constructor : Procedure { NODE_FUNCTIONS_FINAL };

/// A C++ destructor
struct Destructor : Subroutine { NODE_FUNCTIONS_FINAL };

/// This is the type of an array that contains the specified number of elements of the specified type.
struct Array : Type
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Type> element; ///< the element type
    TreePtr<Initialiser> size; ///< evaluates to the size or Uninitialised if not given eg []
};

/// Intermediate for indirect access to some type as specified
struct Indirection : Type
{
    NODE_FUNCTIONS
    TreePtr<Type> destination; ///< reaching an object of this type, indirectly
};

/// A C/C++ pointer
struct Pointer : Indirection { NODE_FUNCTIONS_FINAL };

/// A C++ reference
struct Reference : Indirection { NODE_FUNCTIONS_FINAL };

/// The pseudo-type void, disallowed in some circumstances as per C.
struct Void : Type { NODE_FUNCTIONS_FINAL };

/// Boolean type. 
/** We support bool separately from 1-bit ints, at least for now.
 (note that (bool)2==true but (int:1)2==0)
 Note: Boolean here is considered an adjective, and in general Type
 nodes are named using adjectives. C.f. the Property/Literal intermediate Bool */
struct Boolean : Type { NODE_FUNCTIONS_FINAL };

/// Intermediate for any type that represents a number that you can eg add and subtract. 
struct Numeric : Type { NODE_FUNCTIONS };

/// Type represents an integral (singed or unsigned) type. 
/** The total number of bits (including sign when signed, and 
 fractional if fixed-point) is given */
struct Integral : Numeric
{
    NODE_FUNCTIONS
    TreePtr<Integer> width;  ///< Number of bits, not bytes
};

/// Type of a signed integer number (2's complement).
struct Signed : Integral { NODE_FUNCTIONS_FINAL };

/// Type of an unsigned integer number.
struct Unsigned : Integral { NODE_FUNCTIONS_FINAL };

/// Property for the details of any floating point behaviour
/** implying representation size and implementation. */
struct FloatSemantics : Property { NODE_FUNCTIONS };

/// Property for the details of a specific floating point behaviour
struct SpecificFloatSemantics : FloatSemantics
{
	NODE_FUNCTIONS_FINAL
    SpecificFloatSemantics(); ///< default constructor, for making architypes 
    SpecificFloatSemantics( const llvm::fltSemantics *s ); /// Construct from LLVM's class
	virtual bool IsLocalMatch( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
    virtual CompareResult CovariantCompare( const Matcher *candidate ) const; /// Overloaded comparison for search&replace
	operator const llvm::fltSemantics &() const; /// convert back to LLVM's class
	// TODO no render?
    const llvm::fltSemantics *value;
};    

/// Type of a floating point number.
struct Floating : Numeric 
{ 
	NODE_FUNCTIONS_FINAL
    TreePtr<FloatSemantics> semantics; ///< These are the semantics of the representation
}; 

/// Type of a variable that can hold a label. Similar to the GCC extension
/// for labels-in-variables but we use this type not void * (which is 
/// inconvenient for stataic analysis). To declare a conventional label
/// at a particular position, use Label.
struct Labeley : Type
{
	NODE_FUNCTIONS_FINAL   
};

//////////////////////////// User-defined Types ////////////////////////////

/// Intermediate declaration of a user defined type of any kind (struct, typedef etc).
/* The user type node is a declaration and goes into a declaration scope. It points
 to a TypeIdentifier, and all usages of the type actually point to the
 TypeIdentifier. */
struct UserType : Declaration 
{ 
    NODE_FUNCTIONS
    TreePtr<TypeIdentifier> identifier; ///< The handle to the type that has been declared
};

/// Represents a typedef. 
/** Typedef is to the specified type. We do not expand these at parse, but try to retain
    them for as long as possible */
struct Typedef : UserType
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Type> type; ///< emulate this type
}; 

/// Intermediate for declaration of a struct, class, union or enum. 
/** The set of member Declaration (which will be Field
 or Static) is in the Scope. They can be variables/objects in all 
 cases and additionally Callable instances in Struct/Class. */
struct Record : UserType,
                Scope // Member declarations go in here
{
    NODE_FUNCTIONS
};

/// A union, as per Record.
struct Union : Record { NODE_FUNCTIONS_FINAL };

/// An Enum, as per record. 
/** We regard enumerations as static const variables, initialised as per 
 the given value. Values are always explicit. */
struct Enum : Record { NODE_FUNCTIONS_FINAL };

/// A record that can inherit from other records and be inherited from. 
/** We add in a list of base class declarations. */
struct InheritanceRecord : Record
{
    NODE_FUNCTIONS
    Collection<Base> bases; ///< contains the InheritanceRecords from which we inherit
    // TODO just chuck them into Record::members? 
};

/// Struct as per InheritanceRecord
struct Struct : InheritanceRecord { NODE_FUNCTIONS_FINAL };

/// Class as per InheritanceRecord
struct Class : InheritanceRecord { NODE_FUNCTIONS_FINAL };

//////////////////////////// Expressions ////////////////////////////

/// An operator
//TODO maybe fix the number of operands for binop and unop categories.
struct Operator : Expression
{
	NODE_FUNCTIONS
};

/// An operator with operands whose order is defined
struct NonCommutativeOperator : Operator
{
	NODE_FUNCTIONS
	Sequence<Expression> operands; ///< the operands are here, order preserved
};

/// An operator with operands whose order does not matter
struct CommutativeOperator : Operator
{
	NODE_FUNCTIONS
	Collection<Expression> operands; ///< the operands are here, order not preserved
};

// Intermediate categories of operators. We categorise based on
// topology, and commutative is considered topologically
// distinct from non-commutative.

/// An operator with a single operand
struct Unop : NonCommutativeOperator { NODE_FUNCTIONS };

/// An operator with two operands
struct Binop : NonCommutativeOperator { NODE_FUNCTIONS };

/// An operator with three operands
struct Ternop : NonCommutativeOperator { NODE_FUNCTIONS };

/// An operator with two interchangable operands
struct CommutativeBinop : CommutativeOperator { NODE_FUNCTIONS };

/// An operator with two operands, that writes its result back to the first operand
struct AssignmentOperator : NonCommutativeOperator { NODE_FUNCTIONS };

// Use an include file to generate nodes for all the actual operators based on
// contents of operator_db.inc
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS_FINAL };
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS_FINAL };
#define INFIX(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS_FINAL };
#define OTHER(TOK, TEXT, NODE, BASE, CAT) struct NODE : BASE { NODE_FUNCTIONS_FINAL };
#include "operator_db.inc"

/// Property indicating whether a New/Delete is global 
/** New/Delete is global if it has :: in
 front of it. This differentiates when placement args are given as follows:
 Global: must be one placement arg, it is address to construct at
 NonGlobal: all placement args go to a corresponding operator new which returns address to construct at
 TODO bring these in line with Call etc */
struct Globality : Property { NODE_FUNCTIONS };

/// Property indicating ::new/::delete was used
struct Global : Globality { NODE_FUNCTIONS_FINAL }; 

/// Property indicating just new/delete, no :: was used
struct NonGlobal : Globality { NODE_FUNCTIONS_FINAL }; 

/// Property indicating whether a delete should delete an array.
/** Apologies for the tenuous grammar. */
struct DeleteArrayness : Property { NODE_FUNCTIONS };

/// Property indicating delete[] was used
struct DeleteArray : DeleteArrayness { NODE_FUNCTIONS_FINAL }; 

/// Property indicating delete, no []
struct DeleteNonArray : DeleteArrayness { NODE_FUNCTIONS_FINAL }; 

/// Node for the C++ new operator
/** gives all the syntactical elements required for allocation and initialisation */
struct New : Operator
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Type> type; ///< Type of object to be constructed
    Sequence<Expression> placement_arguments; ///< arguments for placement usage
    Sequence<Expression> constructor_arguments; ///< arguments to the constructor
    TreePtr<Globality> global; ///< whether placement is global
};

/// Node for C++ delete operator
struct Delete : Operator // TODO Statement surely? (clang forces it to be an Expression)
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> pointer; ///< pointer to object to delete
    TreePtr<DeleteArrayness> array; ///< whether array delete
    TreePtr<Globality> global; ///< whether global placement
};

/// Node for accessing an element in a record as in base.member
/** Note that the parser breaks down a->b into (*a).b which may
 be detected using a search pattern if desired. */
struct Lookup : Operator
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> base; ///< the Record we look in
    TreePtr<InstanceIdentifier> member; ///< the member to find
};

/// Node for a c-style cast. 
// TODO C++ casts are not in here yet and C casts will be harmonised into whatever scheme I use for that.
struct Cast : Operator
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> operand; ///< the expression to cast
    TreePtr<Type> type; ///< the desired new type
};

/// Associates an Expression with an InstanceIdentifier. 
/** Basically a key-value pair of identifier and value. Use in Maps. */
struct MapOperand : virtual Node
{
	NODE_FUNCTIONS_FINAL
	TreePtr<InstanceIdentifier> identifier; ///< the handle for this particualar operand
	TreePtr<Expression> value; ///< the Expression for this operand
};

/// An operator with operands whose order is established by mapping
/** Maps a multiplicity of Instances to Expressions via their InstanceIdentifiers.*/
struct MapOperator : Operator
{
	NODE_FUNCTIONS
	Collection<MapOperand> operands; ///< Operands whose relationship is established via identifiers
};

/// A function call to specified function passing in specified arguments
/* Function is an expression to allow eg function pointer dereference. Normal
 calls have callee -> some InstanceIdentifier for a Callable Instance.
 Arguments passed via MapOperator - mapped to the parameters in the callee
 type (if it's a CallableParams). */
struct Call : MapOperator, Uncombable
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> callee; ///< evaluates to the Callable Instance we must call
};

/// Initialiser for a record 
/** Uses a map to associate elements with corresponding record 
 members. We also give the record type explicitly since the map is
 not enough information. */
struct MakeRecord : MapOperator
{
	NODE_FUNCTIONS_FINAL
	TreePtr<TypeIdentifier> type; ///< handle of the type of the record we are making
};

/// Operator that operates on data types as parameters. 
/** Where either is allowed we use the type one, since it's more concise. */
struct TypeOperator : Operator
{
    NODE_FUNCTIONS
    TreePtr<Type> operand; ///< This Type is an input operand 
};

/// sizeof() a type
struct SizeOf : TypeOperator { NODE_FUNCTIONS_FINAL }; 

/// alignof() a type
struct AlignOf : TypeOperator { NODE_FUNCTIONS_FINAL };

//////////////////////////// Statements ////////////////////////////

/// A sequence of statements in a scope that shall execute in sequence
/** Note that local declarations
 can go in the members of the Scope or in the statements (since Declaration
 derives from Statement). There is a sequence point between each statement. */
struct SequentialScope : Scope,
                         virtual Statement
{
    NODE_FUNCTIONS
    Sequence<Statement> statements; ///< Can contain local declarations and code
};

/// Declarations and Statements inside {} or begin/end. 
struct Compound : SequentialScope,      ///< Local declarations go in here (preferably)
                  Initialiser ///< Can "initialise" a function (with the body) 
{
    NODE_FUNCTIONS_FINAL
};                   

/// GCC extension for compound statements that return a value
/** The returned value is that returned by the last statement if it
    is an expresison. Otherwise evaluates to void */
struct CompoundExpression : Expression, ///< Evaluates to whatever the last statement evaluates to
                            SequentialScope       ///< Local declarations go in here (preferably)
{
    NODE_FUNCTIONS_FINAL
};                   

/// The return statement of a function
/** return_value is an Expression giving the return value or 
 Uninitialised if none is present. */
struct Return : Statement
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Initialiser> return_value; ///< return value or Uninitialised
};

/// A goto statement
/** inferno tree supports goto-a-variable because
 it is expected to be useful during sequential lowering (state-out).
 Therefore we do not directly require LabelIdentifier, but the Expression
 must evaluate to one. No * or && needed. */
struct Goto : Statement, Uncombable
{
	NODE_FUNCTIONS_FINAL
    // Dest is an expression for goto-a-variable support.
    // Ordinary gotos will have Label here.
    TreePtr<Expression> destination; ///< where to go to, expresison allowed
};

/// If statement
struct If : Statement
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> condition; ///< condition to test
    TreePtr<Statement> body;       ///< executes when true
    TreePtr<Statement> else_body;  ///< executes when false, can be Nop if no else clause
};

/// Designate a statement that may be broken out of
/** A "break" statement jumps out of the innermost one of these
    and then execution commences immediately after this statement.
    We must specify a body here; the break statement will be 
    within the body */
struct Breakable : Statement 
{
    NODE_FUNCTIONS
    TreePtr<Statement> body; ///< a break in here jumps to the end of here
};

/// Any loop.
/** A "continue" statement jumps out of the innermost one of 
    these and goes to the bottom of the body. So this is effectively
    "Continuable". Our body is inherited from Breakable. */
struct Loop : Breakable { NODE_FUNCTIONS };

/// While loop
struct While : Loop, Uncombable
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> condition; ///< Tested before each iteration; false terminates immediately
};

/// Do loop (first iteration always runs)
struct Do : Loop, Uncombable // a do..while() construct 
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> condition; ///< Tested after each iteration; false terminates immediately
};

/// C-style for loop. 
struct For : Loop
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Statement> initialisation; // Initialiser; use Nop if absent
    TreePtr<Expression> condition;     // Condition; use True if absent
    TreePtr<Statement> increment;      // Increment; use Nop if absent
};

/// Switch statement. 
/** Body (From Breakable) is just a statement scope - case labels
 and breaks are dropped into the sequence at the corresponding 
 positions. This caters for fall-throughs etc. Really just a 
 Compound with a goto-a-variable at the top and some mapping. */
struct Switch : Breakable
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> condition; ///< Evaluates to a value whose case we'll jump to
};

/// Intermediate for labels in a switch statement.
struct SwitchTarget : Statement { NODE_FUNCTIONS };

/// Case label, supporting range extension in case useful for optimisation
struct RangeCase : SwitchTarget
{
	NODE_FUNCTIONS_FINAL
    // support gcc extension of case x..y:
    TreePtr<Expression> value_lo; ///< start of range, inclusive
    TreePtr<Expression> value_hi; ///< end of range, inclusive
}; 

/// Case label
struct Case : SwitchTarget
{
	NODE_FUNCTIONS_FINAL
    TreePtr<Expression> value; ///< Switch jumps here when condition is this value
};

/// Default label in a switch statement
struct Default : SwitchTarget { NODE_FUNCTIONS_FINAL };

/// Continue (to innermost Loop)
struct Continue : Statement, Uncombable { NODE_FUNCTIONS_FINAL };

/// Break (from innermost Breakable)
struct Break : Statement { NODE_FUNCTIONS_FINAL };

/// Do nothing; these get optimised out where possible
struct Nop : Statement { NODE_FUNCTIONS_FINAL };
  
}; // end namespace  
   
#endif

