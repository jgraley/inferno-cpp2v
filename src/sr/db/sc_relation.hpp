#ifndef SC_RELATION_HPP
#define SC_RELATION_HPP

#include "common/common.hpp"
#include "node/specialise_oostd.hpp"
#include "helpers/transformation.hpp"
#include "helpers/walk.hpp"
#include "helpers/flatten.hpp"
#include "../link.hpp"

#include <memory>

class SimpleCompare;

namespace SR
{
class SimpleCompareRelation
{
public:
    SimpleCompareRelation();
    Orderable::Diff Compare( XLink l_xlink, XLink r_xlink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink l_xlink, XLink r_xlink ) const;

    void Test( const unordered_set<XLink> &xlinks );

private:
    shared_ptr<SimpleCompare> simple_compare;
}; 

};

#endif
