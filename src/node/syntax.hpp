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
class Syntax : public virtual Traceable
{
public:
	enum class Production // "Syntax::Production" is a generalisation of precedence
	{		
		PROGRAM = 0,
		BOTTOM_VN,
		BOOT,
		VN_SEP_SCRIPTY, // Separated by ⨟ (in embedded engine renders)
		VN_SEP_ITEMS, // Separated by ⚬ (in StandardAgent renders, which always have ())		
		VN_DESIGNATE_TERM, // Designating non-types using ⪮
		VN_DESIGNATE_TERM_TYPE, // Designating types using ⪮
		VN_DISJUNCTION,
		VN_CONJUNCTION,
		TOP_VN,
						
		// ----- Complete self-sufficient statements and declarations. There is a precedence
		// scheme, booted using {}, to disambiguate if/else.
		BOTTOM_STMT_DECL,		
		MIN_SURR_SEMICOLON = 20, // Note: surroundings higher than here \/ can get ";" added.
		STMT_DECL_LOW, // Lowest statement precedence: {} etc from the inside		
		STMT_DECL_HIGH, // eg if( ... ) <here>;    is this prefix?
		STMT_DECL,
		PRE_PROC_DIRECTIVE, // Preprocessor directives are treated like non-bare statements
		MAX_SURR_SEMICOLON,	// Note: surroundings lower than here /\ can get ";" added.

		// ----- Parts of statements and declarations
		CONDITION = 30,		// Use in surrounds like if( <here> ) ... which could be a decl etc. 
		BRACED,		 // {} from the outside
		LABEL,  // Anything with a : after it. Could be (a) like a bare statement needing a ; or (b) a prefix on statements.
		
		MIN_NODE_SEMICOLON, // Note: nodes higher than here \/ can get ";" added.
		BARE_STMT_DECL = 40, // Statement/declaration with no semicolon
		SPACE_SEP_STMT_DECL, // eg throw <here>;
		TYPE_IN_DECLARATION, // the type in typedef <here>;
		BASE_CLASS_SPEC, // A base class including public etc
		SPACE_SEP_PRE_PROC,
		TOP_STMT_DECL, // Highest statement precedence
						
		// ----- Used in c-style cast. TODO try BOOT.
		BOOT_TYPE = 50,
		
		// ----- Expressions. There is a precedence scheme, booted using ()
		BOTTOM_EXPR = 60, // Lowest expression precedence: (), {}, [] etc from the inside		
		
		COMMA_OP, // Must be lowest binop, so we can bypass it when we want comma as separator
		COMMA_SEP, // Use this for comma-separated lists to force parens for COMMA_OP, including VN
		COLON_SEP, // Use for map arguments

		DECLARATOR_IN_USE, // A type will be rendered using an abstract declarator, may need booting

		ASSIGN = 70, //C++: assign ops and statement-like keywords eg throw
		EXPR_CONST,
		// TODO ?: precedence here,
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
		INITIALISER, // eg MyType thing <here>; prefix = may be inserted. Here to be near ASSIGN	
		PRIMARY_EXPR, // one-token literal eg 12, 12.34, "foo"
		PRIMARY_TYPE, // one-token type eg void, bool
		COMPOUND, // excluded from added ;
		BRACKETED, // (), {} from the outside		
		TOP_EXPR, // Highest expression precedence 
		
		// ----- Abstract, lexer-ish productions of no particuler kind
		EXPLICIT_NODE = 100, // Eg ⯁MyNode⦅...⦆
		RESOLVER, // :: in C++	
		PURE_IDENTIFIER, // Higher than expr because could be a type, label etc
		TERMINAL, // Lexer tokens of any kind (other than type or expression)
		ANONYMOUS,
		NULLPTR // NULL in a pattern as a singluar wildcard
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
			convert_argumentation_to_seq(true),
			boot_statements_using_braces(true),
			compound_uses_vn_separator(false),
			goto_uses_ref_and_deref(true),
			refuse_local_node_types(false),
			full_render_code_unit(true),
			definitions(nullptr),
			can_split_instances(false) {}
		bool force_initialisation;
		bool force_incomplete_records;
		bool split_bulky_statics;
		bool permit_static_keyword;
		// A good thing about typeindex/typeid is that you can refer to a node
		// type without needing to have an actual node of that type.
		type_index current_access;
		bool refuse_c_style_cast;		
		bool detect_and_render_constructor;
		bool convert_argumentation_to_seq;
		bool boot_statements_using_braces;
		bool compound_uses_vn_separator;
		bool goto_uses_ref_and_deref;
		bool refuse_local_node_types;
		bool full_render_code_unit;
	    queue<shared_ptr<Syntax>> *definitions;
		bool can_split_instances;
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
    struct UnimplementedToken : Unimplemented {};
	struct RefusedByPolicy : Refusal {};
	struct RefuseDueLocal : Refusal {};
	struct TemporarilyDisabled : Refusal {};
		
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
    static Production BoostPrecedence( Production prec );
    static int GetPrecedence( Production prec );
    static string::size_type GetLineBreakThreshold();
    static bool IsType( Production prod );
};

#define DEFAULT_NODE_NAMESPACE "CPPTree"

#endif
