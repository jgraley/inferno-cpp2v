#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "common/common.hpp"

/// Interface for being able to reutnr a string for rendering (as opposed to debug)
class Syntax : virtual Traceable
{
public:
	enum class Production // "Syntax::Production" is a generalisation of precedence
	{		
		UNDEFINED,
		
		PROGRAM,
		DEFINITION,		
		
		// ----- Complete self-sufficient statements and declarations. There is a precedence
		// scheme, booted using {}, to disambiguate if/else.
		BOOT_STMT_DECL,		
		STATEMENT_SEQ, // A number of statements in sequence without {}
		STATEMENT_LOW, // Lowest statement precedence: {} etc from the inside		
		STATEMENT_HIGH, // eg if( ... ) <here>;    is this prefix?
		STATEMENT,
		DECLARATION,
		INITIALISER, // eg MyType thing <here>; and " = " is inserted if node is expressional
		BRACED,		 // {} from the outside	#10

		// ----- Parts of statements and declarations
		// Note: surroundings lower than here can get ";" added.
		CONDITION,		// Use in surrounds like if( <here> ) ... which could be a decl etc. 
		PROTOTYPE,      // Render prototype only, no initialiser or ";". 		
		// Note: nodes higher than this can get ";" added.
		BARE_STATEMENT, // Statement with no semicolon
		BARE_DECLARATION, // Actual declaration with no semicolon, eg int i	
		SPACE_SEP_STATEMENT, // eg throw <here>;
		SPACE_SEP_DECLARATION, // the type in <here> <declarator>;
		LABEL,  // Anything with a : after it. Could be (a) like a bare statement needing a ; or (b) a prefix on statements.
		TOP_STMT_DECL, // Highest statement precedence
										
		// ----- Expressions and types. There is a precedence scheme, booted using ()
		BOOT_EXPR, // Lowest expression precedence: (), {}, [] etc from the inside		
		COMMA_OP, // #20
		COMMA_SEP, // Use this for comma-separated lists to force parens for COMMA_OP
		ASSIGN, //C++: assign ops and statement-like keywords eg throw
		LOGIC_OR,
		LOGIC_AND,		
		BITWISE_OR,
		BITWISE_EOR,
		BITWISE_AND,
		EQ_COMP,
		MAG_COMP,
		SHIFT, //  #30
		ADD_SUB,
		MUL_DIV,
		TYPE_IN_NEW, // new(args) <here>(args)
		PREFIX, // C++: all prefix including keywords where expression
		POSTFIX, // C++: all prefix including sub clauses
		SPACE_SEP_TYPE, // eg unsigned long. Interestingly, the spaces take precedence
		PRIMITIVE_EXPR, // one-token literal eg 12, 12.34, "foo"
		PRIMITIVE_TYPE, // one-token type eg void, bool
		PARENTHESISED, // (), {} from the outside		
		TOP_EXPR, // Highest expression precedence
		
		// ----- Abstract, lexer-ish productions of no particuler kind
		SCOPE_RESOLVE, // :: in C++	
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
	
	/// Produce the source-code-name of the corresponding SystemC construct
    virtual string GetToken() const
    {
		return "";
	} 

	virtual string GetRender() const
    {
        return ""; 
    }

	// What production do I become once rendered
	virtual Production GetMyProduction() const
    {
        return Production::UNDEFINED; 
    }
    
    // What production is expected as my operand in a declarator
	virtual Production GetOperandInDeclaratorProduction() const
    {
        return Production::UNDEFINED; 
    }
    
    static Syntax::Production BoostPrecedence( Syntax::Production prec );
    static int GetPrecedence( Syntax::Production prec );
};

#endif
