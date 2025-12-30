#ifndef LOCALTREE_HPP
#define LOCALTREE_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

#define REFUSE_RENDERS \
    string GetRender( VN::RendererInterface *, Production , Policy  ) override \
    { \
		throw RefuseDueLocal(); \
	}


// Nodes that are only used locally to a transformaiton or sequence of transformtions. All
// this is temporary - these are the first candidates to become soft nodes.
namespace LocalTree {

// From Simple C to SC
struct GlobalsModule : SCTree::Module { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };

// From Lower Control Flow 
// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : CPPTree::Switch, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };
struct UncombableFor : CPPTree::For, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };
struct CombableFor : CPPTree::For { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };
struct UncombableBreak : CPPTree::Break, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };
struct CombableBreak : CPPTree::Break { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };

// From Generate stacks
struct TempReturnAddress : CPPTree::Temporary { NODE_FUNCTIONS_FINAL REFUSE_RENDERS };

// From Fall Out
// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : CPPTree::LabelDeclaration
{
    NODE_FUNCTIONS_FINAL 
    REFUSE_RENDERS
    TreePtr<CPPTree::InstanceIdentifier> state;
};


};

#endif
