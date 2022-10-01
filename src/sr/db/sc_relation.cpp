#include "sc_relation.hpp"

#include "helpers/simple_compare.hpp"
#include "helpers/duplicate.hpp"

#include <random>

using namespace SR;

//////////////////////////// SimpleCompareRelation ///////////////////////////////

SimpleCompareRelation::SimpleCompareRelation() :
    simple_compare( make_shared<SimpleCompare>() )
{
}


Orderable::Result SimpleCompareRelation::Compare( XLink xlink, XLink ylink ) const
{
    // Get the child nodes and disregard the arrow heads
    TreePtr<Node> xnode = xlink.GetChildX();
    TreePtr<Node> ynode = ylink.GetChildX();
    
    // And then resort to SimpleCompare
    return simple_compare->Compare( xnode, ynode );
}


bool SimpleCompareRelation::operator()( XLink xlink, XLink ylink ) const
{
    return Compare(xlink, ylink) < Orderable::EQUAL;
}

