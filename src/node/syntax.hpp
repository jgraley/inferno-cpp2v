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
		TRANSLATION_UNIT_CPP,		
		TRANSLATION_UNIT_HPP, 
		
		BOOT_STATEMENT,		
		STATEMENT_LOW, // Lowest statement precedence: {} etc from the inside		
		STATEMENT_HIGH, // eg if( ... ) <here>;    is this prefix?
		STATEMENT,
		DECLARATION,
		BRACED,		 // {} from the outside	#10

		CONDITION,		// eg if( <here> ) ... which could be a decl etc. Surround lower than this can get ";" added.
		PROTOTYPE,      // Render prototype only, no init or ";". Nodes higher than this can get ";" added.
		BARE_STATEMENT, // no trailing ;
		
		SPACE_SEP_STATEMENT, // eg throw <here>;
		SPACE_SEP_DECLARATION, // the type in <here> <declarator>;
		INITIALISER, // For initialisers eg function body		
		LABEL,  // Anything with a : after it. Could be (a) like a bare statement needing a ; or (b) a prefix on statements.

		TOP_STATEMENT, // Highest statement precedence
										
		BOOT_EXPR, // Lowest expression precedence: (), {}, [] etc from the inside		
		COMMA_OP,
		COMMA_SEP, // Use this for comma-separated lists to force parens for COMMA_OP
		ASSIGN, //C++: assign ops and statement-like keywords eg throw
		CONDITIONAL = ASSIGN, 		
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
		PREFIX, // C++: all prefix including keywords where expression
		POSTFIX, // C++: all prefix including sub clauses
		SPACE_SEP_TYPE, // eg auto a = new unsigned long *<here>; - interestingly, the spaces take precedence
		TOKEN, // Highest precedence would be that of lexer tokens
		PARENTHESISED, // (), {} from the outside		
		TOP_EXPR, // Highest expression precedence
		
		SCOPE_RESOLVE, // :: in C++				
		PURE_IDENTIFIER, // Higher than expr because could be a type, label etc
		ANONYMOUS = TOKEN
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
        return GetToken(); 
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
