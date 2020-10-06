#include "equivalence.hpp"

#include "helpers/simple_compare.hpp"

using namespace SR;


EquivalenceRelation::EquivalenceRelation() :
    impl( make_shared<SimpleCompare>() )
{
}


bool EquivalenceRelation::operator()( TreePtr<Node> x, TreePtr<Node> y )
{
    return (*impl)(x, y);
}
