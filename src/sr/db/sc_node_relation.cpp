#include "sc_node_relation.hpp"

#include "helpers/simple_compare.hpp"

using namespace SR;

//////////////////////////// SimpleCompareNodeRelation ///////////////////////////////

SimpleCompareNodeRelation::SimpleCompareNodeRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


Orderable::Result SimpleCompareNodeRelation::Compare( TreePtr<Node> lnode, TreePtr<Node> rnode ) const
{
    return simple_compare->Compare( lnode, rnode );
}


bool SimpleCompareNodeRelation::operator()( TreePtr<Node> lnode, TreePtr<Node> rnode ) const
{
    return Compare(lnode, rnode) < Orderable::EQUAL;
}


Orderable::Result SimpleCompareNodeRelation::Compare( XLink xlink, XLink ylink ) const
{
    // Get the child nodes and disregard the arrow heads
    TreePtr<Node> xnode = xlink.GetChildX();
    TreePtr<Node> ynode = ylink.GetChildX();
    
    // And then resort to SimpleCompare
    return Compare( xnode, ynode );
}


bool SimpleCompareNodeRelation::operator()( XLink xlink, XLink ylink ) const
{
    return Compare(xlink, ylink) < Orderable::EQUAL;
}

