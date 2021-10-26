#ifndef CANNONICALISER_HPP
#define CANNONICALISER_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "link.hpp"

#include <memory>

namespace SR
{

class Cannonicaliser : public InPlaceTransformation
{
public:
    virtual void operator()( TreePtr<Node> context,     // The whole program, so declarations may be searched for
    		                 TreePtr<Node> *proot );    // Root of the subtree we want to modify
    
private:
    int PassOneWalk( TreePtr<Node> node );
    void Uniquify( const set< TreePtr<Node> > &nodes, 
                   set< TreePtr<Node> > &uniques );
    void RedirectParentPointers( TreePtr<Node> to_be_overwritten, TreePtr<Node> by );
    
    // Filled in during pass 1
    map< TreePtr<Node>, list< FlattenNode::iterator > > parent_pointers;
    vector< set< TreePtr<Node> > > nodes_by_shallowness;
    
    // Filled in during pass 2
    vector< set< TreePtr<Node> > > uniques_by_shallowness;    
};

};

#endif
