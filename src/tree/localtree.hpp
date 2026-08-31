#ifndef LOCALTREE_HPP
#define LOCALTREE_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

#define RENDER_AS_BASE_IN_CPP_ONLY(BASE) \
    string GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy ) override \
    { \
		if( policy.refuse_local_nodes_without_overridden_syntax ) \
			throw RefuseDueLocal(); /* Produce full explicit node */ \
		return BASE::GetRender(renderer, surround_prod, policy); \
	} \
	YY::VNLangParser::token::token_kind_type GetKeywordToken() const override \
	{ \
		throw UnimplementedToken(); \
	}
		

// See #899 about using this macro
#define KEYWORD_AS_BASE_IN_CPP_ONLY(BASE) \
    string GetKeyword(Policy policy) const override \
    { \
		if( policy.refuse_local_nodes_without_overridden_syntax ) \
			throw UnimplementedKeyword(); /* Produce short-form explicit node */ \
		return BASE::GetKeyword(policy); \
	} \
	YY::VNLangParser::token::token_kind_type GetKeywordToken() const override \
	{ \
		throw UnimplementedToken(); \
	}

// Nodes that are only used locally to a transformaiton or sequence of transformtions. All
// this is temporary - these are the first candidates to become soft nodes.
namespace LocalTree {

// From Simple C to SC
struct GlobalsModule : SCTree::Module { NODE_FUNCTIONS_FINAL };

// From Lower Control Flow 
// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : CPPTree::Switch, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::Switch) };
struct UncombableFor : CPPTree::For, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::For) };
struct CombableFor : CPPTree::For { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::For) };
struct UncombableBreak : CPPTree::Break, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::Break) };
struct CombableBreak : CPPTree::Break { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::Break) };

// From Generate stacks
struct TempReturnAddress : CPPTree::Temporary { NODE_FUNCTIONS_FINAL RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::Temporary) };

// From Fall Out
// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : CPPTree::LabelDeclaration
{
    NODE_FUNCTIONS_FINAL 
    RENDER_AS_BASE_IN_CPP_ONLY(CPPTree::LabelDeclaration)
    TreePtr<CPPTree::InstanceIdentifier> state;
};


};

#endif
