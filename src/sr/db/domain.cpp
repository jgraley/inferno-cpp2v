#include "../sc_relation.hpp"
#include "../agents/agent.hpp"

#include "domain.hpp"

using namespace SR;    

void Domain::Clear()
{
    unordered_domain.clear();
    domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
}    


void Domain::SetOnExtraXLinkFunction( OnExtraXLinkFunction on_extra_xlink_ )
{
    on_extra_xlink = on_extra_xlink_;
}


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


void Domain::ExtendDomainWorker( const TreeKit &kit, PatternLink plink )
{
    // Extend locally first and then pass that into children.
    set<XLink> extra_xlinks = plink.GetChildAgent()->ExpandNormalDomain( kit, unordered_domain );    
    if( !extra_xlinks.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");
    for( XLink extra_xlink : extra_xlinks )
        on_extra_xlink( extra_xlink );
    
    // Visit couplings repeatedly TODO union over couplings and
    // only recurse on last reaching.
    auto pq = plink.GetChildAgent()->GetPatternQuery();    
    for( PatternLink child_plink : pq->GetNormalLinks() )
    {
        ExtendDomainWorker( kit, child_plink );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomainWorker( kit, child_plink );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomainWorker( kit, child_plink );
    }
}


void Domain::ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink_ )
{
	root_plink = root_plink_;
	
    ExtendDomainWorker(kit, root_plink);
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due to new pattern ")(root_plink)(", new XLinks:\n")
             ( DifferenceOf(unordered_domain, previous_unordered_domain) )
             ("\nRemoved XLinks:\n")
             ( DifferenceOf(previous_unordered_domain, unordered_domain) )("\n");
        previous_unordered_domain = unordered_domain;
    }
#endif
}


void Domain::ExtendDomainNewX(const TreeKit &kit)
{
    ExtendDomainWorker(kit, root_plink);
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due new X, pattern is ")(root_plink)(", new XLinks:\n")
             ( DifferenceOf(unordered_domain, previous_unordered_domain) )
             ("\nRemoved XLinks:\n")
             ( DifferenceOf(previous_unordered_domain, unordered_domain) )("\n");
        previous_unordered_domain = unordered_domain;
    }
#endif
}


void Domain::PopulateActions( DBWalk::Actions &actions )
{
	actions.domain_in_is_ok = [&](const DBWalk::WalkInfo &walk_info) -> bool
	{
		return unordered_domain.count( walk_info.xlink ) == 0;
	};
	
	actions.domain_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Update domain
		InsertSolo( unordered_domain, walk_info.xlink );
		
		// Here, elements go into quotient set, but it does not 
		// uniquify: every link in the input X tree must appear 
		// separately in domain.
		(void)domain_extension_classes->Uniquify( walk_info.xlink );    
	};
}


void Domain::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	PopulateActions( actions );
}


void Domain::PrepareIncrementalInsert(DBWalk::Actions &actions)
{
}


