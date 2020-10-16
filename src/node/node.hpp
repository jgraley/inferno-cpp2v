#ifndef NODE_HPP
#define NODE_HPP

#include "common/common.hpp"
#include "specialise_oostd.hpp"
#include "itemise.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"
#include "match.hpp"
#include "renderable.hpp"

#define FINAL_FUNCTION(F) virtual bool IsFinal() { return (F); }

// Mix together the bounce classes for the benefit of the tree
// TODO figure out how to enforce finality in NODE_FUNCTIONS_FINAL
#define NODE_FUNCTIONS ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(false)
#define NODE_FUNCTIONS_FINAL ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(true)
/// The node support classes all collected together for convenience
struct NodeBases : Magic,
                   virtual Renderable,
                   Matcher,
                   Itemiser,
                   Cloner,
                   SerialNumber
{
    string GetAddr() const
    {
        return SerialNumber::GetAddr(); // avoiding the need for virtual inheritance
    }
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
struct Node : NodeBases
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
};


template<>
struct MakeTreePtr<Node> : TreePtr<Node>
{
	MakeTreePtr() : TreePtr<Node>( new Node ) {}
	template<typename CP0>
	MakeTreePtr(const CP0 &cp0) : TreePtr<Node>( new Node(cp0) ) {}
	template<typename CP0, typename CP1>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<Node>( new Node(cp0, cp1) ) {}
	template<typename CP0, typename CP1, typename CP2>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<Node>( new Node(cp0, cp1, cp2) ) {}
	// Add more params as needed...
};


extern void GenericsTest();

#endif
