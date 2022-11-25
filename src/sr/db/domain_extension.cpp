#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "helpers/duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

using namespace SR;    


DomainExtension::ExtenderSet DomainExtension::DetermineExtenders( const set<const SYM::Expression *> &sub_exprs )
{
	ExtenderSet extenders;
    for( auto sub_expr : sub_exprs )
    {
        if( auto ext = dynamic_cast<const Extender *>(sub_expr) )
        { 
            extenders.insert( ext );
        }
    }

    return extenders;
}


DomainExtension::DomainExtension( const XTreeDatabase *db_, ExtenderSet extenders_ ) :
    db( db_ ),
    extenders(extenders_) 
{
	FTRACE(extenders);
}
	

void DomainExtension::SetOnExtraXLinkFunctions( OnExtraZoneFunction on_insert_extra_zone_,
                                                OnExtraZoneFunction on_delete_extra_zone_ )
{
    on_insert_extra_zone = on_insert_extra_zone_;
    on_delete_extra_zone = on_delete_extra_zone_;
}


XLink DomainExtension::GetUniqueDomainExtension( TreePtr<Node> node ) const
{   
    ASSERT( node );

	ASSERT( domain_extension_classes.count(node) > 0 )
	      (node)(" not found in domain_extension_classes:\n")
	      (domain_extension_classes);

    return domain_extension_classes.at( node );
}


void DomainExtension::ExtendDomainBaseXLink( const TreeKit &kit, TreePtr<Node> node )
{
    ASSERT( node );

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
}


void DomainExtension::ExtendDomainPatternWalk( const TreeKit &kit, PatternLink plink )
{
    // Extend locally first and then pass that into children.
    // This avoids the need for a reductive "keep trying until no more
    // extra XLinks are provided" because we know that only the child pattern
    // can match a pattern node's generated XLink.
    set<TreePtr<Node>> extend_nodes = plink.GetChildAgent()->ExpandNormalDomain( kit, db->GetDomain().unordered_domain );      
    if( !extend_nodes.empty() )
        TRACE("There are extra x domain elements for ")(plink)(":\n");

    for( TreePtr<Node> node : extend_nodes )
        ExtendDomainBaseXLink( kit, node );
    
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


void DomainExtension::ExtendDomainNewPattern( const TreeKit &kit, PatternLink root_plink_ )
{
	root_plink = root_plink_;
    ExtendDomainPatternWalk(kit, root_plink);
}


void DomainExtension::PrepareDelete( DBWalk::Actions &actions )
{
	actions.domain_extension_out = [=](const DBWalk::WalkInfo &walk_info)
	{        
		(void)domain_extension_classes.erase( walk_info.x );    
	};
}


void DomainExtension::PrepareInsert(DBWalk::Actions &actions)
{
	actions.domain_extension_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
		(void)domain_extension_classes.insert( make_pair( walk_info.x, walk_info.xlink ) );    
	};
}


void DomainExtension::PrepareDeleteExtra( DBWalk::Actions &actions )
{
	actions.domain_extension_out = [=](const DBWalk::WalkInfo &walk_info)
	{        
        // TODO probably erases the class too soon - would need to keep a count of the number of
        // elements or something and only erase when it hits zero. But there my be bigger fish to fry here.
		(void)domain_extension_classes.erase( walk_info.x );    
	};
}


void DomainExtension::PrepareInsertExtra(DBWalk::Actions &actions)
{
	actions.domain_extension_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
        // Not solo because domain_extension_classes is not a total ordering- 
        // there may already be a class for this xlink
		(void)domain_extension_classes.insert( make_pair( walk_info.x, walk_info.xlink ) );    
	};
}


bool DomainExtension::Relation::operator() (TreePtr<Node> l_node, TreePtr<Node> r_node) const
{
    return Compare3Way( l_node, r_node ) < 0;
}


Orderable::Diff DomainExtension::Relation::Compare3Way(TreePtr<Node> l_node, TreePtr<Node> r_node) const
{
    ASSERT( l_node );
    ASSERT( r_node );
    return sc.Compare3Way( *l_node, *r_node );
}


void DomainExtension::Relation::Test( const unordered_set<XLink> &xlinks )
{
    // We do not expect stability and totality in this relation WRT any given type:
    // - SC is tighter than base node value since it looks at whole subtree by value
    // - TreePtr is tighter than SC because it looks at the subtree by identity
    // - XLink is tighter still, because it looks at TreePtr by identity
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


void DomainExtension::TestRelations( const unordered_set<XLink> &xlinks )
{	
    DomainExtension::Relation dr;
    dr.Test( xlinks );
    
    // Exposes #688
    //TestOrderingIntact( domain_extension_classes,
    //                    false,
    //                    "domain_extension_classes" );
}


