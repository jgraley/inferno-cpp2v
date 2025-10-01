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
		TRANSLATION_UNIT,
		
		INSTANCE_PROTO, 
		
		INITIALISER, // For initialisers
		
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
		PREFIX, // C++: all prefix including keywords where expression
		POSTFIX, // C++: all prefix including sub clauses
		SCOPE_RESOLVE, // :: in C++
		DESTRUCTOR_MARK, // The ~ prefix on destructors
		
		PARENTHESISED, // Highest expression precedence: (), {} from the outside
		
		PURE_IDENTIFIER,
		TOKEN, // Highest precedence would be that of lexer tokens
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
