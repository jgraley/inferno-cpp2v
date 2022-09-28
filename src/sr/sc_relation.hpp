#ifndef SC_RELATION_HPP
#define SC_RELATION_HPP

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
class SimpleCompareRelation
{
public:
    SimpleCompareRelation();
    Orderable::Result Compare( XLink xlink, XLink ylink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink xlink, XLink ylink ) const;
    
    /// Perform a test to check the reflexive, symmetric and 
    /// transitive properties, given a representative set
    /// of links. Pass if no assert failure.
    void TestProperties( const set<XLink> &xlinks ) const;
private:
    shared_ptr<SimpleCompare> simple_compare;
}; 

};

#endif
