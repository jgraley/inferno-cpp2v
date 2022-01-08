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
    Orderable::Result Compare( XLink xlink, XLink ylink ) const;

    /// Less operator: for use with set, map etc
    bool operator()( XLink xlink, XLink ylink ) const;
    
    /// Perform a test to check the reflexiv, symmetric and 
    /// transitive properties, given a representative set
    /// of links. Pass if no assert failure.
    void TestProperties( const set<XLink> &xlinks ) const;
private:
    shared_ptr<SimpleCompare> simple_compare;
}; 


class QuotientSet
{
public:
    XLink Uniquify( XLink x );
    void Clear();
    
private:    
    typedef set<XLink, EquivalenceRelation> Classes;
    Classes classes;
};

};

#endif
