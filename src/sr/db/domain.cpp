#include "domain.hpp"

#include "relation_test.hpp"

#include "../agents/agent.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    

Domain::Domain()
{
}
	

void Domain::SetOnExtraXLinkFunctions( OnExtraXLinkFunction on_insert_extra_subtree_,
                                       OnExtraXLinkFunction on_delete_extra_xlink_ )
{
    on_insert_extra_subtree = on_insert_extra_subtree_;
    on_delete_extra_xlink = on_delete_extra_xlink_;
}


XLink Domain::UniquifyDomainExtension( TreePtr<Node> node, bool expect_in_domain )
{
    ASSERT( node );
  
    // If there's already a class for this node, return it and early-out
    auto it = domain_extension_classes.find( node );
    if( it != domain_extension_classes.end() )
        return it->second;
  
    // There was not a class, which could be an error
    ASSERT( !expect_in_domain );
  
    // Not an error, so create an XLink that will allow us to track this subtree
    XLink xlink = XLink::CreateDistinct( node );    
  
    // And insert it
    auto p = domain_extension_classes.insert( make_pair( node, xlink ) );
    ASSERT( p.second ); // false if was already there, contradicting the find() above
    
    return xlink;
}


void Domain::ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink, bool remove )
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
        ExtendDomainPatternWalk( kit, child_plink, remove );
    }
    for( PatternLink child_plink : pq->GetAbnormalLinks() )
    {
        ExtendDomainPatternWalk( kit, child_plink, remove );
    }
    for( PatternLink child_plink : pq->GetMultiplicityLinks() )
    {
        ExtendDomainPatternWalk( kit, child_plink, remove );
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
    TRACE("Domain extensions believed to be:\n")(extended_domain)("\n");
#endif    

    unordered_set<XLink> to_delete = extended_domain;
    for( auto it = extended_domain.begin(); it != extended_domain.end(); )
    {
        on_delete_extra_xlink( *it );
        it = extended_domain.erase( it );
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


void Domain::PrepareMonolithicBuild(DBWalk::Actions &actions, bool extra)
{
	actions.is_unreached = [&](const DBWalk::WalkInfo &walk_info) -> bool
	{
		return unordered_domain.count( walk_info.xlink ) == 0;
	};
	
	actions.domain_in = [&](const DBWalk::WalkInfo &walk_info)
	{        
		// ----------------- Update domain
		InsertSolo( unordered_domain, walk_info.xlink ); 
        if( extra )
            InsertSolo( extended_domain, walk_info.xlink );
        
#ifdef TRACE_DOMAIN_EXTEND
        TRACE("Saw domain extra ")(walk_info.xlink)(" extra flag=")(extra)(" ud.size=%u ed.size()=%u\n", unordered_domain.size(), extended_domain.size());
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


