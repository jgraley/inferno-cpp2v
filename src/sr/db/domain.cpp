#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../agents/agent.hpp"
#include "../vn_step.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "tables.hpp"

#include "domain.hpp"

using namespace SR;    

XLink Domain::UniquifyDomainExtension( XLink xlink )
{
    ASSERT( xlink );
  
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Uniquify( xlink ); 
}


XLink Domain::FindDomainExtension( XLink xlink ) const
{
    ASSERT( xlink );
    
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( unordered_domain.count(xlink) > 0 )
        return xlink;
        
    return domain_extension_classes->Find( xlink ); 
}


void Domain::ExtendDomainWorker( XTreeDatabase *db, PatternLink plink )
{
    // Extend locally first and then pass that into children.
    set<XLink> extra_xlinks = plink.GetChildAgent()->ExpandNormalDomain( *db, unordered_domain );    
    if( !extra_xlinks.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");
    for( XLink extra_xlink : extra_xlinks )
    {
        TRACEC(extra_xlink)("\n");
        db->GetTables().AddAtRoot( Tables::STOP_IF_ALREADY_IN, extra_xlink ); // set to REQUIRE_SOLO to replicate #218
    }
    
    // Visit couplings repeatedly TODO union over couplings and
    // only recurse on last reaching.
    auto pq = plink.GetChildAgent()->GetPatternQuery();    
    for( PatternLink child_plink : pq->GetNormalLinks() )
    {
        ExtendDomainWorker( db, child_plink );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomainWorker( db, child_plink );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomainWorker( db, child_plink );
    }
}


void Domain::ExtendDomainNewPattern( XTreeDatabase *db, PatternLink root_plink_ )
{
	root_plink = root_plink_;
	
    ExtendDomainWorker(db, root_plink);
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due to new pattern ")(root_plink)(", new XLinks:\n")
             ( DifferenceOf(category_ordered_index, previous_category_ordered_domain) )
             ("\nRemoved XLinks:\n")
             ( DifferenceOf(previous_category_ordered_domain, category_ordered_index) )("\n");
        previous_category_ordered_domain = category_ordered_index;
    }
#endif

#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    SimpleCompareRelation e;
    e.TestProperties( unordered_domain );
#endif
}


void Domain::ExtendDomainNewX(XTreeDatabase *db)
{
    ExtendDomainWorker(db, root_plink);
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due new X, pattern is ")(root_plink)(", new XLinks:\n")
             ( DifferenceOf(category_ordered_index, previous_category_ordered_domain) )
             ("\nRemoved XLinks:\n")
             ( DifferenceOf(previous_category_ordered_domain, category_ordered_index) )("\n");
        previous_category_ordered_domain = category_ordered_index;
    }
#endif

#ifdef TEST_RELATION_PROPERTIES_USING_DOMAIN    
    SimpleCompareRelation e;
    e.TestProperties( unordered_domain );
#endif
}
