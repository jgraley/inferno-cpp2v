#ifndef EQUIVALENCE_HPP
#define EQUIVALENCE_HPP

#include "common/common.hpp"
#include "node/node.hpp"

#include <memory>

class SimpleCompare;

namespace SR
{
// At present we only have one equivalence relation
class EquivalenceRelation
{
public:
    EquivalenceRelation();
    bool operator()( TreePtr<Node> x, TreePtr<Node> y );

private:
    shared_ptr<SimpleCompare> impl;
}; 

};

#endif
