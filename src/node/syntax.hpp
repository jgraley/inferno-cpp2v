#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "common/common.hpp"
#include "common/mismatch.hpp"

/// Interface for being able to reutnr a string for rendering (as opposed to debug)
class Syntax : virtual Traceable
{
public:
	enum class Production // "Syntax::Production" is a generalisation of precedence
	{		
		UNDEFINED,
		
		PROGRAM = 10,
		DEFINITION,		
		
		// ----- Complete self-sufficient statements and declarations. There is a precedence
		// scheme, booted using {}, to disambiguate if/else.
		BOOT_STMT_DECL = 20,		
		STATEMENT_SEQ, // A number of statements in sequence without {}
		STATEMENT_LOW, // Lowest statement precedence: {} etc from the inside		
		STATEMENT_HIGH, // eg if( ... ) <here>;    is this prefix?
		STATEMENT,
		DECLARATION,
		INITIALISER, // eg MyType thing <here>; and " = " is inserted if node is expressional
		BRACED,		 // {} from the outside

		// ----- Parts of statements and declarations
		// Note: surroundings lower than here can get ";" added.
		CONDITION = 30,		// Use in surrounds like if( <here> ) ... which could be a decl etc. 
		PROTOTYPE,      // Render prototype only, no initialiser or ";". 		
		// Note: nodes higher than this can get ";" added.
		BARE_STATEMENT = 40, // Statement with no semicolon
		BARE_DECLARATION, // Actual declaration with no semicolon, eg int i	
		SPACE_SEP_STATEMENT, // eg throw <here>;
		SPACE_SEP_DECLARATION, // the type in <here> <declarator>;
		LABEL,  // Anything with a : after it. Could be (a) like a bare statement needing a ; or (b) a prefix on statements.
		TOP_STMT_DECL, // Highest statement precedence
										
		// ----- Expressions and types. There is a precedence scheme, booted using ()
		BOOT_EXPR = 50, // Lowest expression precedence: (), {}, [] etc from the inside		

		VN_SEP_SCRIPTY, // Separated by ⨟ (in embedded engine renders)
		VN_SEP_ITEMS, // Separated by ⚬ (in StandardAgent renders, which always have ())
		
		COMMA_OP, 
		COMMA_SEP, // Use this for comma-separated lists to force parens for COMMA_OP, including VN

		VN_DESIGNATE, // defining names using ⪮
		VN_DISJUNCTION,
		VN_CONJUNCTION,

		ASSIGN = 60, //C++: assign ops and statement-like keywords eg throw
		LOGIC_OR,
		LOGIC_AND,		
		BITWISE_OR,
		BITWISE_EOR,
		BITWISE_AND,
		EQ_COMP,
		MAG_COMP,
		SHIFT, 
		ADD_SUB,
		MUL_DIV,
		TYPE_IN_NEW, // new(args) <here>(args)
		PREFIX, // C++ and VN: all prefix 
		POSTFIX, // C++: all postfix including sub clauses
		SPACE_SEP_TYPE, // eg unsigned long. Interestingly, the spaces take precedence
		PRIMITIVE_EXPR, // one-token literal eg 12, 12.34, "foo"
		PRIMITIVE_TYPE, // one-token type eg void, bool
		BRACKETED, // (), {} from the outside		
		TOP_EXPR, // Highest expression precedence
		
		// ----- Abstract, lexer-ish productions of no particuler kind
		SCOPE_RESOLVE = 80, // :: in C++	
		PURE_IDENTIFIER, // Higher than expr because could be a type, label etc
		TOKEN, // Lexer tokens of any kind (other than type or expression)
		ANONYMOUS
	};
	
	// We deal with syntactical association only, not mathematical, because:
	// - most operators can be overloaded with no-associative stuff and/or side-effects
	// - most of the others are side-effecty eg &&, ||, ?:
	enum class Association
	{
		LEFT,
		RIGHT
	};
	
	struct NotOnThisNode : Exception
	{
	};
	
	/// Produce the source-code-name of the corresponding SystemC construct
    virtual string GetLoweredIdName() const;
    virtual string GetIdentifierName() const;
    virtual string GetCouplingNameHint() const;
	virtual string GetRenderTerminal() const;
	virtual Production GetMyProduction() const;
	virtual Production GetOperandInDeclaratorProduction() const;
    static Syntax::Production BoostPrecedence( Syntax::Production prec );
    static int GetPrecedence( Syntax::Production prec );
    static string::size_type GetLineBreakThreshold();
};

#endif
