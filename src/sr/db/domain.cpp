#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

//#define BYPASS

using namespace SR;    

Domain::Domain()
{
}
	

void Domain::SetOnExtraXLinkFunctions( OnExtraZoneFunction on_insert_extra_zone_,
                                       OnExtraZoneFunction on_delete_extra_zone_ )
{
    on_insert_extra_zone = on_insert_extra_zone_;
    on_delete_extra_zone = on_delete_extra_zone_;
}


XLink Domain::GetUniqueDomainExtension( Agent::TeleportResult tpr ) const
{
    ASSERT( tpr.second );
  
#ifdef BYPASS
    if( unordered_domain.count(tpr.second) > 0 )
		return xlink;
#endif		
  
    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    return domain_extension_classes.at( tpr.second );
}


void Domain::ExtendDomainBaseXLink( const TreeKit &kit, Agent::TeleportResult tpr )
{
    TreePtr<Node> node = tpr.second;
    ASSERT( node );
  
#ifdef BYPASS
    if( unordered_domain.count(node) > 0 )
        return; // Is meaningful
#endif

    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    if( domain_extension_classes.count(node) > 0 )
        return; // Is conincidental
        
    // An extra subtree is required
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> extra_node = Duplicate::DuplicateSubtree( node );
  
    // Create an XLink that will allow us to track this subtree
    XLink extra_xlink = XLink::CreateDistinct( extra_node );    
  
    // Make a zone. 
    auto extra_zone = TreeZone(extra_xlink);
    ASSERT( !extra_zone.IsEmpty() ); 

#ifdef TRACE_DOMAIN_EXTEND
    TRACE("Zone is ")(extra_zone)("\n"); 
#endif    
        
    // Add this domain extension to the whole database including
    // our domain_extension_classes
    on_insert_extra_zone( extra_zone );        
    
    // Ensure the original tree is found in the domain now (it wasn't 
    // earlier on) as an extra check
    ASSERT( domain_extension_classes.count( node ) == 1 );
    
    // Remember we did this so UnExtendDomain() can undo it
    // TODO should we push to front for a LIFO action?
    extra_zones.push_back( extra_zone ); // TODO std::move the zone
}


void Domain::ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink )
{
    // Extend locally first and then pass that into children.
    // This avoids the need for a reductive "keep trying until no more
    // extra XLinks are provided" because we know that only the child pattern
    // can match a pattern node's generated XLink.
    set<Agent::TeleportResult> tp_results = plink.GetChildAgent()->ExpandNormalDomain( kit, unordered_domain );      
    if( !tp_results.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");

    for( Agent::TeleportResult tpr : tp_results )
        ExtendDomainBaseXLink( kit, tpr );
    
    // Visit couplings repeatedly TODO union over couplings and
    // only recurse on last reaching.
    auto pq = plink.GetChildAgent()->GetPatternQuery();    
    for( PatternLink child_plink : pq->GetNormalLinks() )
    {
        ExtendDomainPatternWalk( kit, child_plink );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomainPatternWalk( kit, child_plink );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomainPatternWalk( kit, child_plink );
    }
}


void Domain::ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink_ )
{
	root_plink = root_plink_;
	
#ifdef TRACE_DOMAIN_EXTEND
	unordered_set<XLink> previous_unordered_domain = unordered_domain;
#endif    

    ExtendDomainPatternWalk(kit, root_plink);

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

    ExtendDomainPatternWalk(kit, root_plink);

#ifdef TRACE_DOMAIN_EXTEND
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain extended due new X, pattern is ")(root_plink)(", domain change is:\n")
             ( DiffTrace(previous_unordered_domain, unordered_domain) );
    }
#endif
}


void Domain::UnExtendDomain()
{
#ifdef TRACE_DOMAIN_EXTEND
	unordered_set<XLink> previous_unordered_domain = unordered_domain;
    TRACE("Domain extensions believed to be:\n")(extra_zones)("\n"); 
#endif    

    for( auto it = extra_zones.begin(); it != extra_zones.end(); )
    {
        on_delete_extra_zone( *it );
        it = extra_zones.erase( it );
    }

	ASSERT( extra_zones.empty() );

#ifdef TRACE_DOMAIN_EXTEND
    if( Tracer::IsEnabled() ) // We want this deltaing to be relative to what is seen in the log
    {
        TRACE("Domain un-extended pattern is ")(root_plink)(", domain change is:\n")
             ( DiffTrace(previous_unordered_domain, unordered_domain) );
    }
#endif
}


void Domain::PrepareDelete( DBWalk::Actions &actions )
{
	actions.domain_out = [=](const DBWalk::WalkInfo &walk_info)
	{        
        // TODO probably erases the class too soon - would need to keep a count of the number of
        // elements or something and only erase when it hits zero. But there my be bigger fish to fry here.
		(void)domain_extension_classes.erase( walk_info.x );    

		EraseSolo( unordered_domain, walk_info.xlink );
	};
}


void Domain::PrepareInsert(DBWalk::Actions &actions)
{
	actions.domain_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
        // Not solo because domain_extension_classes is not a total ordering- 
        // there may already be a class for this xlink
		(void)domain_extension_classes.insert( make_pair( walk_info.x, walk_info.xlink ) );    

		InsertSolo( unordered_domain, walk_info.xlink );   
	};
}


bool Domain::Relation::operator() (TreePtr<Node> l_node, TreePtr<Node> r_node) const
{
    return Compare3Way( l_node, r_node ) < 0;
}


Orderable::Diff Domain::Relation::Compare3Way(TreePtr<Node> l_node, TreePtr<Node> r_node) const
{
    ASSERT( l_node );
    ASSERT( r_node );
    return sc.Compare3Way( *l_node, *r_node );
}


void Domain::Relation::Test( const unordered_set<XLink> &xlinks )
{
    // We do not expect stability and totality in this relation WRT any given type:
    // - SC is tighter than base node value since it looks at whole subtree by value
    // - TreePtr is tighter than SC because it looks at the subtree by identity
    // - XLink is tighter still, becausae it looks at TreePtr by identity
    // If we have to choose between stbility and totality, we'll choose stability.
    // We don't actually need totality because these are equivalence classes.
    SimpleCompare sc;
	TestRelationProperties( xlinks,
                            false,
                            "SimpleCompare (Domain)",
                            function<string()>(),
    [&](XLink l, XLink r)
    { 
        return sc.Compare3Way(l.GetChildX(), r.GetChildX()); 
    }, 
	[&](XLink l, XLink r)
    { 
        return l.GetChildX()==r.GetChildX(); 
    } );
}


void Domain::TestRelations( const unordered_set<XLink> &xlinks )
{	
    Domain::Relation dr;
    dr.Test( xlinks );
}


