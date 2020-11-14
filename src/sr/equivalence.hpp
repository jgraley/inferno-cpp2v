#ifndef EQUIVALENCE_HPP
#define EQUIVALENCE_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "link.hpp"

#include <memory>

class SimpleCompare;

namespace SR
{
// At present we only have one equivalence relation
class EquivalenceRelation
{
public:
    EquivalenceRelation();
    CompareResult Compare( XLink xlink, XLink ylink );

    /// Less operator: for use with set, map etc
    bool operator()( XLink xlink, XLink ylink );
    
private:
    shared_ptr<SimpleCompare> simple_compare;
}; 


class QuotientSet
{
public:
    XLink GetQuotient( XLink x );
    
private:    
    typedef set<XLink, EquivalenceRelation> Classes;
    Classes classes;
};


};


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

#endif
