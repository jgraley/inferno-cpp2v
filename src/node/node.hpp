#ifndef NODE_HPP
#define NODE_HPP

#include "common/common.hpp"
#include "itemise.hpp"
#include "clone.hpp"
#include "common/magic.hpp"
#include "match.hpp"
#include "renderable.hpp"
#include "graphable.hpp"
#include "coloured.hpp"
#include "relationship.hpp"
#include "invented.hpp"
#include "common/serial.hpp"
#include "common/orderable.hpp"

#include <memory>

#define FINAL_FUNCTION(F) virtual bool IsFinal() const { return (F); }

// Mix together the bounce classes for the benefit of the tree
// TODO figure out how to enforce finality in NODE_FUNCTIONS_FINAL
#define NODE_FUNCTIONS ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(false)
#define NODE_FUNCTIONS_FINAL ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(true)
/// The node support classes all collected together for convenience
struct NodeBases : Magic,
                   virtual Renderable,
                   virtual Graphable,
                   Coloured,
                   Matcher,
                   Orderable,
                   Itemiser,
                   Cloner,
                   SerialNumber,                   
                   ChildRelationship,
                   Invented
{
};

class SequenceInterface;
class CollectionInterface;
class TreePtrInterface;

// Base class for all tree nodes and nodes in search/replace
// patterns etc. Convention is to use "struct" for derived
// node classes so that everything is public (inferno tree nodes
// are more like records in a database, they have only minimal
// functionality). Also, all derived structs should contain the
// NODE_FUNCTIONS macro which expands to a few virtual functions
// required for common ("bounced") functionality. Where multiple
// inheritance diamonds arise, Node should be derived virtually
// (we always want the set-restricting model of inheritance in
// the inferno tree node hierarchy).
/// The main base class for inferno nodes
struct Node : NodeBases,
              enable_shared_from_this<Node>
{
    // C++11 fix
    Node& operator=(Node& other)
    {
        // Must do nothing - will get invoked mutliple times due
        // virtual MI diamonds
        return *this;
    }

    NODE_FUNCTIONS

    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds
    // without making Node ambiguous
    
    virtual string GetGraphName() const
    {
        return GetRender();
    }
    
    virtual Graphable::Block GetGraphBlockInfo() const override;
    
    // It's nice to share. So we try to make these available to other 
    // graphables, such as agents
    static list<Graphable::SubBlock> GetSubblocks( SequenceInterface *seq, 
                                                   Phase phase=Graphable::UNDEFINED );
    static list<Graphable::SubBlock> GetSubblocks( CollectionInterface *col, 
                                                   Phase phase=Graphable::UNDEFINED );
    static list<Graphable::SubBlock> GetSubblocks( const TreePtrInterface *singular, 
                                                   Phase phase=Graphable::UNDEFINED );
                                                   
    virtual string GetGraphId() const override;    
    virtual string GetTrace() const override;
};


extern void GenericsTest();

#endif
