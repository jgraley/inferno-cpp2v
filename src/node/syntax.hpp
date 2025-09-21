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
		
		INITIALISER, // For initialisers
		
		BOOT_EXPR, // Lowest precedence is called "BOOT_EXPR"
		
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
		SCOPE_RES, // :: in C++
		
		PARENTHESISED, // enclosed in (), {} etc
		
		TOKEN // Highest precedence would be that of lexer tokens
	};
	
	// We deal with syntactical association only, not mathematical, because:
	// - most operators can be overloaded with no-associative stuff and/or side-effects
	// - most of the others are side-effecty eg &&, ||, ?:
	enum class Association
	{
		LEFT,
		RIGHT
	};
	
	virtual string GetRender() const
    {
        // If no render supplied just return the name - right for graphs
        // and a debugging aid in C++ renders
        return GetName(); 
    }

	virtual Production GetIdealProduction() const
    {
        // If no render supplied just return the name - right for graphs
        // and a debugging aid in C++ renders
        return Production::UNDEFINED; 
    }
    
    static Syntax::Production BoostPrecedence( Syntax::Production prec );
    static int GetPrecedence( Syntax::Production prec );
};

#endif
