#ifndef LOCALTREE_HPP
#define LOCALTREE_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

#define RENDER_AS_BASE_IN_CPP_ONLY(BASE) \
    string GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy ) override \
    { \
		if( policy.permit_inherited_keyword ) \
			return BASE::GetRender(renderer, surround_prod, policy); \
		throw RefuseDueLocal(); /* Produce full explicit node */ \
	} \
	YY::VNLangParser::token::token_kind_type GetKeywordToken() const override \
	{ \
		throw UnimplementedToken(); \
	}
		

// See #899 about using this macro
#define KEYWORD_AS_BASE_IN_CPP_ONLY(BASE, TOKEN) \
    string GetKeyword(Policy policy) const override \
    { \
		if( policy.permit_inherited_keyword ) \
			return BASE::GetKeyword(policy); \
		throw UnimplementedKeyword(); /* Produce short-form explicit node */ \
	} \
	YY::VNLangParser::token::token_kind_type GetKeywordToken() const override \
	{ \
		return TOKEN; \
	}

// Nodes that are only used locally to a transformaiton or sequence of transformtions. All
// this is temporary - these are the first candidates to become soft nodes.
namespace LocalTree {

// From Simple C to SC
struct GlobalsModule : SCTree::Module { NODE_FUNCTIONS_FINAL };

// From LowerControlFlow 
// These work with short-form explicits
struct UncombableSwitch : CPPTree::Switch, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL KEYWORD_AS_BASE_IN_CPP_ONLY(CPPTree::Switch, YY::VNLangParser::token::TOK_KEYWORD_CONTROL_STMT) };
struct UncombableFor : CPPTree::For, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL KEYWORD_AS_BASE_IN_CPP_ONLY(CPPTree::For, YY::VNLangParser::token::TOK_KEYWORD_CONTROL_STMT) };
struct CombableFor : CPPTree::For { NODE_FUNCTIONS_FINAL KEYWORD_AS_BASE_IN_CPP_ONLY(CPPTree::For, YY::VNLangParser::token::TOK_KEYWORD_CONTROL_STMT) };
struct UncombableBreak : CPPTree::Break, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL KEYWORD_AS_BASE_IN_CPP_ONLY(CPPTree::Break, YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT) };
struct CombableBreak : CPPTree::Break { NODE_FUNCTIONS_FINAL KEYWORD_AS_BASE_IN_CPP_ONLY(CPPTree::Break, YY::VNLangParser::token::TOK_KEYWORD_SIMPLE_STMT) };

// From GenerateStacks
// Requires long-form explicit because no keyword to override
// TODO #902 switch to short-form explicit
struct TempReturnAddress : CPPTree::Temporary { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::Temporary) };

// From Fall Out
// Requires long-form explicit because no keyword to override and no syntax for specifying state
// TODO #903 for short-form explicit
struct StateLabel : CPPTree::LabelDeclaration
{
    NODE_FUNCTIONS_FINAL 
    RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::LabelDeclaration)
    TreePtr<CPPTree::InstanceIdentifier> state;
};


};

#endif
