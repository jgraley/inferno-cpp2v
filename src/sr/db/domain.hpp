#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../link.hpp"
#include "common/standard.hpp"

#include <unordered_set>
#include <functional>

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
	typedef function<void(XLink)> OnExtraXLinkFunction;

	void SetOnExtraXLinkFunction( OnExtraXLinkFunction on_extra_xlink );

    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink UniquifyDomainExtension( XLink xlink );

    // Get the cannonical xlink for the given one.
    XLink FindDomainExtension( XLink xlink ) const;
    
    void ExtendDomainWorker( const TreeKit &kit, PatternLink plink );
    void ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink );
    void ExtendDomainNewX( const TreeKit &kit );

    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<SimpleCompareQuotientSet> domain_extension_classes;

private:
    OnExtraXLinkFunction on_extra_xlink;
  	PatternLink root_plink;
};    
    
}

#endif
