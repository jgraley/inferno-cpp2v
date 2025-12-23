#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "common/common.hpp"
#include "common/mismatch.hpp"

#include <typeinfo>
#include <typeindex>

namespace VN
{
	class RendererInterface;
};

/// Interface for being able to reutnr a string for rendering (as opposed to debug)
class Syntax : virtual Traceable
{
public:
	enum class Production // "Syntax::Production" is a generalisation of precedence
	{		
		BOOT = 0,
		PROGRAM,
		VN_SEP_SCRIPTY, // Separated by ⨟ (in embedded engine renders)
		VN_SEP_ITEMS, // Separated by ⚬ (in StandardAgent renders, which always have ())		
		VN_DESIGNATE, // defining names using ⪮
		
		// ----- Complete self-sufficient statements and declarations. There is a precedence
		// scheme, booted using {}, to disambiguate if/else.
		BOTTOM_STMT_DECL,		
		MIN_SURR_SEMICOLON = 20, // Note: surroundings higher than here \/ can get ";" added.
		STATEMENT_SEQ, // A number of statements in sequence without {}
		STATEMENT_LOW, // Lowest statement precedence: {} etc from the inside		
		STATEMENT_HIGH, // eg if( ... ) <here>;    is this prefix?
		STATEMENT,
		DECLARATION,
		PRE_PROC_DIRECTIVE, // Preprocessor directives are treated like non-bare statements
		MAX_SURR_SEMICOLON,	// Note: surroundings lower than here /\ can get ";" added.

		// ----- Parts of statements and declarations
		CONDITION = 30,		// Use in surrounds like if( <here> ) ... which could be a decl etc. 
		BRACED,		 // {} from the outside
		
		MIN_NODE_SEMICOLON, // Note: nodes higher than here \/ can get ";" added.
		BARE_STATEMENT = 40, // Statement with no semicolon
		BARE_DECLARATION, // Actual declaration with no semicolon, eg int i	
		SPACE_SEP_STATEMENT, // eg throw <here>;
		SPACE_SEP_DECLARATION, // the type in <here> <declarator>;
		SPACE_SEP_PRE_PROC,
		LABEL,  // Anything with a : after it. Could be (a) like a bare statement needing a ; or (b) a prefix on statements.
		TOP_STMT_DECL, // Highest statement precedence

		INITIALISER = 50, // eg MyType thing <here>; and " = " is inserted if node is expressional
							
		// ----- Types. Just this for now.
		BOOT_TYPE,
		TOP_TYPE,
		
		// ----- Expressions. There is a precedence scheme, booted using ()
		BOTTOM_EXPR = 60, // Lowest expression precedence: (), {}, [] etc from the inside		
		
		COMMA_OP, // Must be lowest binop, so we can bypass it when we want comma as separator
		COMMA_SEP, // Use this for comma-separated lists to force parens for COMMA_OP, including VN
		COLON_SEP, // Use for map arguments

		VN_DISJUNCTION,
		VN_CONJUNCTION,

		ASSIGN = 70, //C++: assign ops and statement-like keywords eg throw
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
		PRIMARY_EXPR, // one-token literal eg 12, 12.34, "foo"
		PRIMARY_TYPE, // one-token type eg void, bool
		BRACKETED, // (), {} from the outside		
		TOP_EXPR, // Highest expression precedence 
		
		// ----- Abstract, lexer-ish productions of no particuler kind
		EXPLICIT_NODE = 90, // Eg ◼CPPTree::MyNode(...)
		RESOLVER, // :: in C++	
		PURE_IDENTIFIER, // Higher than expr because could be a type, label etc
		TERMINAL, // Lexer tokens of any kind (other than type or expression)
		ANONYMOUS
	};
	
	static const type_index DefaultAccess;
	
	struct Policy
	{
		Policy() : 
			force_initialisation(false), 
			force_incomplete_records(false),
			split_bulky_statics(false),
			permit_static_keyword(false),
			current_access(DefaultAccess),
			refuse_c_style_cast(false),
			detect_and_render_constructor(true),
			refuse_call_if_map_args(true),
			boot_statements_using_braces(true) {}
		bool force_initialisation;
		bool force_incomplete_records;
		bool split_bulky_statics;
		bool permit_static_keyword;
		// A good thing about typeindex/typeid is that you can refer to a node
		// type without needing to have an actual node of that type.
		type_index current_access;
		bool refuse_c_style_cast;		
		bool detect_and_render_constructor;
		bool refuse_call_if_map_args;
		bool boot_statements_using_braces;
	};
	
	// We deal with syntactical association only, not mathematical, because:
	// - most operators can be overloaded with no-associative stuff and/or side-effects
	// - most of the others are side-effecty eg &&, ||, ?:
	enum class Association
	{
		LEFT,
		RIGHT
	};
	
	struct Refusal : Exception {};
	struct Unimplemented : Refusal {};
	struct RefusedByPolicy : Refusal {};
		
	/// Produce the source-code-name of the corresponding SystemC construct
    virtual string GetLoweredIdName() const;
    virtual string GetIdentifierName() const;
    virtual string GetDesignationNameHint() const;
    virtual bool IsDesignationNamedIdentifier() const;
	
	virtual string GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy );
	
	// Like GetRender, but without a kit it can't render sub-productions, so it can only work for terminals
	virtual string GetRenderTerminal( Production surround_prod ) const;
	
	virtual Production GetMyProduction(const VN::RendererInterface *renderer, Policy policy ) const;
	virtual Production GetMyProductionTerminal() const;
	virtual Production GetOperandInDeclaratorProduction() const;
    static Syntax::Production BoostPrecedence( Syntax::Production prec );
    static int GetPrecedence( Syntax::Production prec );
    static string::size_type GetLineBreakThreshold();
};

#endif
