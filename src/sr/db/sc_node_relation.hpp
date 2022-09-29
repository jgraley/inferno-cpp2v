#ifndef SC_NODE_RELATION_HPP
#define SC_NODE_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "../link.hpp"

#include <memory>

class SimpleCompare;

namespace SR
{
class SimpleCompareNodeRelation
{
public:
    SimpleCompareNodeRelation();
    Orderable::Result Compare( TreePtr<Node> lnode, TreePtr<Node> rnode ) const;

    /// Less operator: for use with set, map etc
    bool operator()( TreePtr<Node> xnode, TreePtr<Node> rnode ) const;
    
    // Compatibility functions
    Orderable::Result Compare( XLink xlink, XLink ylink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink xlink, XLink ylink ) const;
    
private:
    shared_ptr<SimpleCompare> simple_compare;
}; 

};

#endif
