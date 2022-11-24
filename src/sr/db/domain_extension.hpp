#ifndef DOMAIN_EXTENSION_HPP
#define DOMAIN_EXTENSION_HPP

#include "../zone.hpp"
#include "../link.hpp"
#include "helpers/simple_compare.hpp"
#include "common/standard.hpp"
#include "db_walk.hpp"
#include "node/specialise_oostd.hpp"
#include "../agents/agent.hpp"

#include <unordered_set>
#include <functional>

class SimpleCompare;
    
/// SR namespace contains the search and replace implementation
namespace SR 
{
    
class DomainExtension
{   
public:
	DomainExtension();
	
	typedef function<void(const TreeZone &)> OnExtraZoneFunction;

	void SetOnExtraXLinkFunctions( OnExtraZoneFunction on_insert_extra_zone,
                                   OnExtraZoneFunction on_delete_extra_zone = OnExtraZoneFunction() );

    // Add xlink to domain extension if not already there, and return the cannonical one.
    XLink GetUniqueDomainExtension( TreePtr<Node> node ) const; 
    
    void ExtendDomainBaseXLink( const TreeKit &kit, TreePtr<Node> node );
    void ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink );
    void ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink );

    void PrepareDelete(DBWalk::Actions &actions);
	void PrepareInsert(DBWalk::Actions &actions);
    void PrepareDeleteExtra(DBWalk::Actions &actions);
	void PrepareInsertExtra(DBWalk::Actions &actions);

    void TestRelations( const unordered_set<XLink> &xlinks );
    
    class Relation
    {
    public:
        bool operator() (TreePtr<Node> l_node, TreePtr<Node> r_node) const;
        Orderable::Diff Compare3Way(TreePtr<Node> l_node, TreePtr<Node> r_node) const;
        
        void Test( const unordered_set<XLink> &xlinks );
    private:
        SimpleCompare sc;
    };
    
    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // SimpleCompare equivalence classes over the domain.
    map<TreePtr<Node>, XLink, SimpleCompare> domain_extension_classes;

private:
    OnExtraZoneFunction on_insert_extra_zone;
    OnExtraZoneFunction on_delete_extra_zone;
    
  	PatternLink root_plink;  	
};    
    
}

#endif
