#ifndef DOMAIN_HPP
#define DOMAIN_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "sc_node_relation.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "node/specialise_oostd.hpp"

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
    
class Domain
{   
public:
	Domain();
	
	typedef function<void(XLink)> OnExtraXLinkFunction;

	void SetOnExtraXLinkFunctions( OnExtraXLinkFunction on_insert_extra_subtree,
                                   OnExtraXLinkFunction on_delete_extra_xlink = OnExtraXLinkFunction() );

    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink UniquifyDomainExtension( TreePtr<Node> node, bool expect_in_domain );

    // Get the cannonical xlink for the given one.
    XLink FindDomainExtension( TreePtr<Node> node ) const;
    
    void ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink, bool remove = false );
    void ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink );
    void ExtendDomainNewX( const TreeKit &kit );
    void UnExtendDomain();

    void MonolithicClear();
	void PrepareMonolithicBuild(DBWalk::Actions &actions, bool extra);
    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);

    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // SimpleCompare equivalence classes over the domain.
    map<TreePtr<Node>, XLink, SimpleCompareNodeRelation> domain_extension_classes;

private:
    OnExtraXLinkFunction on_insert_extra_subtree;
    OnExtraXLinkFunction on_delete_extra_xlink;
  	PatternLink root_plink;
    unordered_set<XLink> extended_domain;
};    
    
}

#endif
