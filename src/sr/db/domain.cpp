#include "../sc_relation.hpp"
#include "../agents/agent.hpp"

#include "domain.hpp"

#define TRACE_DOMAIN_EXTEND


using namespace SR;    

Domain::Domain() :
	domain_extension_classes( make_shared<SimpleCompareQuotientSet>() )
{
}
	

void Domain::SetOnExtraXLinkFunctions( OnExtraXLinkFunction on_insert_extra_subtree_,
                                       OnExtraXLinkFunction on_delete_extra_xlink_ )
{
    on_insert_extra_subtree = on_insert_extra_subtree_;
    on_delete_extra_xlink = on_delete_extra_xlink_;
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


void Domain::PatternWalker( const TreeKit &kit, PatternLink plink, bool remove )
{
    // Extend locally first and then pass that into children.
    // This avoids the need for a reductive "keep trying until no more
    // extra XLinks are provided" because we know that only the child pattern
    // can match a pattern node's generated XLink.
    set<XLink> extra_subtrees = plink.GetChildAgent()->ExpandNormalDomain( kit, unordered_domain );    
    if( !extra_subtrees.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");
    for( XLink extra_base_xlink : extra_subtrees )
        on_insert_extra_subtree( extra_base_xlink );
    
    // Visit couplings repeatedly TODO union over couplings and
    // only recurse on last reaching.
    auto pq = plink.GetChildAgent()->GetPatternQuery();    
    for( PatternLink child_plink : pq->GetNormalLinks() )
    {
        PatternWalker( kit, child_plink, remove );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        PatternWalker( kit, child_plink, remove );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        PatternWalker( kit, child_plink, remove );
    }
}


void Domain::ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink_ )
{
	root_plink = root_plink_;
	
#ifdef TRACE_DOMAIN_EXTEND
	unordered_set<XLink> previous_unordered_domain = unordered_domain;
#endif    

    PatternWalker(kit, root_plink);

#ifdef TRACE_DOMAIN_EXTEND
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due to new pattern ")(root_plink)(", domain change is:\n")
             ( DiffTrace(previous_unordered_domain, unordered_domain) );
    }
#endif
}


void Domain::ExtendDomainNewX(const TreeKit &kit)
{
#ifdef TRACE_DOMAIN_EXTEND
	unordered_set<XLink> previous_unordered_domain = unordered_domain;
#endif    

    PatternWalker(kit, root_plink);

#ifdef TRACE_DOMAIN_EXTEND
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due new X, pattern is ")(root_plink)(", domain change is:\n")
             ( DiffTrace(previous_unordered_domain, unordered_domain) );
    }
#endif
}


void Domain::UnExtendDomain(const TreeKit &kit)
{
#ifdef TRACE_DOMAIN_EXTEND
	unordered_set<XLink> previous_unordered_domain = unordered_domain;
#endif    

    for( XLink extra_xlink : extended_domain )
    {
        extended_domain.erase( extra_xlink );
        on_delete_extra_xlink( extra_xlink );
    }

#ifdef TRACE_DOMAIN_EXTEND
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain un-extended pattern is ")(root_plink)(", domain change is:\n")
             ( DiffTrace(previous_unordered_domain, unordered_domain) );
    }
#endif
}


void Domain::MonolithicClear()
{
    unordered_domain.clear();
    extended_domain.clear();
    domain_extension_classes->Clear();
}    


void Domain::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.is_unreached = [&](const DBWalk::WalkInfo &walk_info) -> bool
	{
		return unordered_domain.count( walk_info.xlink ) == 0;
	};
	
	actions.domain_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Update domain
		InsertSolo( unordered_domain, walk_info.xlink ); 
        InsertSolo( extended_domain, walk_info.xlink );
        
        // Here, elements go into quotient set, but it does not 
		// uniquify: every link in the input X tree must appear 
		// separately in domain.
		(void)domain_extension_classes->Uniquify( walk_info.xlink );    
	};
}


void Domain::PrepareDelete( DBWalk::Actions &actions )
{
}


void Domain::PrepareInsert(DBWalk::Actions &actions)
{
}


