#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"

#include <unordered_set>
#include <functional>

//#define TRACE_X_TREE_DB_DELTAS

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
	Domain();
	
	typedef function<void(XLink)> OnExtraXLinkFunction;

	void SetOnExtraXLinkFunction( OnExtraXLinkFunction on_extra_xlink );

    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink UniquifyDomainExtension( XLink xlink );

    // Get the cannonical xlink for the given one.
    XLink FindDomainExtension( XLink xlink ) const;
    
    void ExtendDomainWorker( const TreeKit &kit, PatternLink plink );
    void ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink );
    void ExtendDomainNewX( const TreeKit &kit );

    void MonolithicClear();
	void PrepareMonolithicBuild(DBWalk::Actions &actions);
    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);

    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
#ifdef TRACE_X_TREE_DB_DELTAS
	unordered_set<XLink> previous_unordered_domain;
#endif    

    // SimpleCompare equivalence classes over the domain.
    shared_ptr<SimpleCompareQuotientSet> domain_extension_classes;

private:
    OnExtraXLinkFunction on_extra_xlink;
  	PatternLink root_plink;
};    
    
}

#endif
