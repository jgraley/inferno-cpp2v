#ifndef LOCALTREE_HPP
#define LOCALTREE_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"

// Nodes that are only used locally to a transformaiton or sequence of transformtions. All
// this is temporary - these are the first candidates to become soft nodes.
namespace LocalTree {

// From Simple C to SC
struct GlobalsModule : SCTree::Module { NODE_FUNCTIONS_FINAL };

// From Lower Control Flow 
// Local nodes let us designate switch and for nodes as uncombable
struct UncombableSwitch : CPPTree::Switch, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL };
struct UncombableFor : CPPTree::For, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL };
struct CombableFor : CPPTree::For { NODE_FUNCTIONS_FINAL };
struct UncombableBreak : CPPTree::Break, CPPTree::Uncombable { NODE_FUNCTIONS_FINAL };
struct CombableBreak : CPPTree::Break { NODE_FUNCTIONS_FINAL };

// From Generate stacks
struct TempReturnAddress : CPPTree::Temporary { NODE_FUNCTIONS_FINAL };

// From Fall Out
// A label with a piggybacked pointer to the corresponding enum value
struct StateLabel : CPPTree::Label
{
    NODE_FUNCTIONS_FINAL
    TreePtr<CPPTree::InstanceIdentifier> state;
};


};

#endif
