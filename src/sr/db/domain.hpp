#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "../sc_relation.hpp"
#include "helpers/simple_compare.hpp"
#include "helpers/transformation.hpp"
#include "tables.hpp"

#include <unordered_set>

class SimpleCompare;

namespace SYM
{
    class BooleanExpression;
};
    
/// SR namespace contains the search and replace implementation
namespace SR 
{
class SimpleCompareQuotientSet;
class VNStep;
class Lacing;
class XTreeDatabase;
    
class Domain
{   
public:
    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink UniquifyDomainExtension( XLink xlink );

    // Get the cannonical xlink for the given one.
    XLink FindDomainExtension( XLink xlink ) const;
    
    void ExtendDomainWorker( XTreeDatabase *db, PatternLink plink );
    void ExtendDomainNewPattern( XTreeDatabase *db, PatternLink root_plink );
    void ExtendDomainNewX( XTreeDatabase *db );

    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<SimpleCompareQuotientSet> domain_extension_classes;

private:
  	PatternLink root_plink;
};    
    
}

#endif
