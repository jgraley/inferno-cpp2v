#ifndef LOCALTREE_HPP
#define LOCALTREE_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

#define RENDER_MACRO(BASE) \
    string GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy ) override \
    { \
		if( policy.refuse_local_node_types ) \
			throw RefuseDueLocal(); \
		return BASE::GetRender(renderer, surround_prod, policy); \
	}


// Nodes that are only used locally to a transformaiton or sequence of transformtions. All
// this is temporary - these are the first candidates to become soft nodes.
namespace LocalTree {

// From Simple C to SC
struct GlobalsModule : SCTree::Module { NODE_FUNCTIONS_FINAL RENDER_MACRO(SCTree::Module) };

// From Lower Control Flow 
// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : CPPTree::Switch, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::Switch) };
struct UncombableFor : CPPTree::For, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::For) };
struct CombableFor : CPPTree::For { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::For) };
struct UncombableBreak : CPPTree::Break, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::Break) };
struct CombableBreak : CPPTree::Break { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::Break) };

// From Generate stacks
struct TempReturnAddress : CPPTree::Temporary { NODE_FUNCTIONS_FINAL RENDER_MACRO(CPPTree::Temporary) };

// From Fall Out
// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : CPPTree::LabelDeclaration
{
    NODE_FUNCTIONS_FINAL 
    RENDER_MACRO(CPPTree::LabelDeclaration)
    TreePtr<CPPTree::InstanceIdentifier> state;
};


};

#endif
