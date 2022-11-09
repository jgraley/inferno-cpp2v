#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"
#include "helpers/duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

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


XLink Domain::UniquifyDomainExtension( TreePtr<Node> node, bool expect_in_domain )
{
    ASSERT( node );
  
    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    auto it = domain_extension_classes.find( node );
    
    if( expect_in_domain )
        ASSERT( it != domain_extension_classes.end() );
    
    // If in domain, we're done
    if( it != domain_extension_classes.end() )
        return it->second;
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> dup_node = Duplicate::DuplicateSubtree( node );
  
    // Not an error, so create an XLink that will allow us to track this subtree
    XLink xlink = XLink::CreateDistinct( dup_node );    
  
    // And insert it
    auto p = domain_extension_classes.insert( make_pair( dup_node, xlink ) );
    ASSERT( p.second ); // false if was already there, contradicting the find() above
    
    // Ensure the original tree is found in the domain now (it wasn't earlier on)
    // as an extra check
    ASSERT( domain_extension_classes.count( node ) == 1 );
    
    return xlink;
}


void Domain::ExtendDomainBaseXLink( const TreeKit &kit, XLink base_xlink )
{
    auto zone = TreeZone::CreateFromExclusions(base_xlink, unordered_domain );
#ifdef TRACE_DOMAIN_EXTEND
    TRACE("Zone is ")(zone)("\n"); 
#endif    
    if( zone.IsEmpty() )
        return;
        
    on_insert_extra_zone( zone );        
    extended_zones.push_back( zone ); // TODO std::move the zone
}


void Domain::ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink )
{
    // Extend locally first and then pass that into children.
    // This avoids the need for a reductive "keep trying until no more
    // extra XLinks are provided" because we know that only the child pattern
    // can match a pattern node's generated XLink.
    set<XLink> subtrees = plink.GetChildAgent()->ExpandNormalDomain( kit, unordered_domain );    
    if( !subtrees.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");
    for( XLink base_xlink : subtrees )
        ExtendDomainBaseXLink( kit, base_xlink );
    
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
    TRACE("Domain extensions believed to be:\n")(extended_zones)("\n"); 
#endif    

    for( auto it = extended_zones.begin(); it != extended_zones.end(); )
    {
        on_delete_extra_zone( *it );
        it = extended_zones.erase( it );
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
    domain_extension_classes.clear();
}    


void Domain::PrepareDeleteMonolithic(DBWalk::Actions &actions)
{
	actions.domain_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
		EraseSolo( unordered_domain, walk_info.xlink ); 
        
#ifdef TRACE_DOMAIN_EXTEND
		TRACE("Saw xlink ")(walk_info.xlink)(" ud.size=%u ed.size()=%u\n", unordered_domain.size(), extended_zones.size());
#endif
        
        // TODO probably erases the class too soon - would need to keep a count of the number of
        // elements or something and only erase when it hits zero. But there my be bigger fish to fry here.
		(void)domain_extension_classes.erase( walk_info.xlink.GetChildX() );    
	};
}


void Domain::PrepareInsertMonolithic(DBWalk::Actions &actions)
{
	actions.domain_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
		InsertSolo( unordered_domain, walk_info.xlink ); 
        
#ifdef TRACE_DOMAIN_EXTEND
		TRACE("Saw xlink ")(walk_info.xlink)(" ud.size=%u ed.size()=%u\n", unordered_domain.size(), extended_zones.size());
#endif
        
		(void)domain_extension_classes.insert( make_pair( walk_info.xlink.GetChildX(), walk_info.xlink ) );    
	};
}


void Domain::PrepareDelete( DBWalk::Actions &actions )
{
}


void Domain::PrepareInsert(DBWalk::Actions &actions)
{
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


