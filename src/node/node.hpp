#ifndef NODE_HPP
#define NODE_HPP

//#define NODE_IS_GRAPHABLE

#include "common/common.hpp"
#include "itemise.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"
#include "match.hpp"
#include "renderable.hpp"
#ifdef NODE_IS_GRAPHABLE
#include "graphable.hpp"
#endif
#include "coloured.hpp"
#include "common/serial.hpp"

#include <memory>

#define FINAL_FUNCTION(F) virtual bool IsFinal() { return (F); }

// Mix together the bounce classes for the benefit of the tree
// TODO figure out how to enforce finality in NODE_FUNCTIONS_FINAL
#define NODE_FUNCTIONS ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(false)
#define NODE_FUNCTIONS_FINAL ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(true)
/// The node support classes all collected together for convenience
struct NodeBases : Magic,
                   virtual Renderable,
#ifdef NODE_IS_GRAPHABLE
                   virtual Graphable,
#endif
                   Coloured,
                   Matcher,
                   Itemiser,
                   Cloner,
                   SerialNumber
{
};

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
    
#ifdef NODE_IS_GRAPHABLE
    Graphable::Block GetGraphBlockInfo( const LinkNamingFunction &lnf );
#endif
};


extern void GenericsTest();

#endif
