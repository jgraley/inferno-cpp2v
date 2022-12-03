#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "../agents/teleport_agent.hpp"
#include "helpers/duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

#define TRUE_DE

using namespace SR;    

// ------------------------- DomainExtension --------------------------

bool DomainExtension::ExtenderClassRelation::operator()( const Extender *l, const Extender *r ) const
{
	return l->IsExtenderLess( *r );
}


DomainExtension::ExtenderSet DomainExtension::DetermineExtenders( const set<const SYM::Expression *> &sub_exprs )
{
	ExtenderSet extenders;
    for( auto sub_expr : sub_exprs )
    {
        if( auto tp_op = dynamic_cast<const TeleportAgent::TeleportOperator *>(sub_expr) )
        { 
            extenders.insert( tp_op->GetAgent() );
        }
    }

    return extenders;
}


DomainExtension::DomainExtension( const XTreeDatabase *db, ExtenderSet extenders )
{
	for( const Extender *extender : extenders )
	     channels[extender] = make_unique<DomainExtensionChannel>(db, extender);
}
	

void DomainExtension::SetOnExtraXLinkFunctions( OnExtraZoneFunction on_insert_extra_zone_,
                                                OnExtraZoneFunction on_delete_extra_zone_ )
{
	for( auto &p : channels )
		p.second->SetOnExtraXLinkFunctions( on_insert_extra_zone_, on_delete_extra_zone_ );
}


XLink DomainExtension::GetUniqueDomainExtension( const Extender *extender, TreePtr<Node> node ) const
{   
    ASSERT( channels.count(extender) );
    return channels.at(extender)->GetUniqueDomainExtension( node );
}


void DomainExtension::InitialBuild()
{
	for( auto &p : channels )
        p.second->InitialBuild();
}


void DomainExtension::Complete()
{
	for( auto &p : channels )
        p.second->Complete();
}


void DomainExtension::PrepareDelete( DBWalk::Actions &actions )
{
	actions.domain_extension_out = [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->Delete( walk_info );
	};
}


void DomainExtension::PrepareInsert(DBWalk::Actions &actions)
{
	actions.domain_extension_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->Insert( walk_info );			 
	};
}


void DomainExtension::PrepareDeleteExtra( DBWalk::Actions &actions )
{
	actions.domain_extension_out = [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->DeleteExtra( walk_info );			
	};
}


void DomainExtension::PrepareInsertExtra(DBWalk::Actions &actions)
{
	actions.domain_extension_in = [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->InsertExtra( walk_info );			 
	};
}


void DomainExtension::TestRelations( const unordered_set<XLink> &xlinks )
{	
}

// ------------------------- DomainExtensionChannel --------------------------

DomainExtensionChannel::DomainExtensionChannel( const XTreeDatabase *db_, const DomainExtension::Extender *extender_ ) :
    db( db_ ),
    extender( extender_ )
{
}


void DomainExtensionChannel::SetOnExtraXLinkFunctions( DomainExtension::OnExtraZoneFunction on_insert_extra_zone_,
                                                       DomainExtension::OnExtraZoneFunction on_delete_extra_zone_ )
{
    on_insert_extra_zone = on_insert_extra_zone_;
    on_delete_extra_zone = on_delete_extra_zone_;
}


XLink DomainExtensionChannel::GetUniqueDomainExtension( TreePtr<Node> node ) const
{   
    ASSERT( node );

	ASSERT( domain_extension_classes.count(node) > 0 )
	      (node)(" not found in domain_extension_classes:\n")
	      (domain_extension_classes);

    return domain_extension_classes.at( node ).extra_xlink;
}


void DomainExtensionChannel::ExtendDomainExtraNode( TreePtr<Node> extra_node )
{
    ASSERT( extra_node );

    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    if( domain_extension_classes.count(extra_node) > 0 )
    {
        domain_extension_classes.at(extra_node).count++;
        return; 
    }
        
    // An extra subtree is required
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> extra_node_dup = Duplicate::DuplicateSubtree( extra_node );
  
    // Create an XLink that will allow us to track this subtree
    XLink extra_xlink = XLink::CreateDistinct( extra_node_dup );    
  
    // Make a zone. 
    auto extra_zone = TreeZone(extra_xlink);
    ASSERT( !extra_zone.IsEmpty() ); 

#ifdef TRACE_DOMAIN_EXTEND
    TRACE("Zone is ")(extra_zone)("\n"); 
#endif    
        
    // Add this xlink to the extension classes as start. Count starts 
    // at 1 since there's one ref (this one)
	(void)domain_extension_classes.insert( make_pair( extra_node_dup, ExtClass(extra_xlink, 1) ) );    
        
    // Add the whole zone to the rest of the database
    on_insert_extra_zone( extra_zone );        
    
    // Ensure the original tree is found in the domain now (it wasn't 
    // earlier on) as an extra check
    ASSERT( domain_extension_classes.count( extra_node ) == 1 );
}


void DomainExtensionChannel::ExtendDomain( XLink start_xlink )
{
    set<XLink> deps;
    TreePtr<Node> extra_node = extender->GetDomainExtraNode( db, start_xlink, deps );  
    if( !extra_node )
        return;
    
    start_to_tracking.insert( make_pair( start_xlink, TrackingRow(extra_node, deps) ) );
    for( XLink dep : deps )
        dep_to_starts[dep].insert(start_xlink);
    
    ExtendDomainExtraNode( extra_node );
}


void DomainExtensionChannel::DropStartXlink( XLink start_xlink )
{
    // Be strict here: all these data structures need to remain in synch
    ASSERT( start_to_tracking.count(start_xlink)>0 );
    
    TreePtr<Node> extra_node = start_to_tracking.at(start_xlink).extra_node;
    set<XLink> &deps = start_to_tracking.at(start_xlink).deps;

    // Remove this starting xlink from deps structures, possibly dropping the
    // dep completely
    for( XLink dep : deps )
    {
        ASSERT( dep_to_starts.count(dep)>0 );
        ASSERT( !dep_to_starts.at(dep).empty() );
        EraseSolo(dep_to_starts.at(dep), start_xlink);
        if( dep_to_starts.at(dep).empty() )
            EraseSolo(dep_to_starts, dep);
    }

    // Remove this starting link from domain extension classes, possibly
    // dropping the extnesion class completely.
    ASSERT( domain_extension_classes.count(extra_node) > 0 );
    int nc = --domain_extension_classes.at(extra_node).count;
    if( nc==0 )
        EraseSolo( domain_extension_classes, extra_node );

    // Remove tracking row for this starting xlink
    EraseSolo(start_to_tracking, start_xlink);
}


void DomainExtensionChannel::Validate() const
{
    for( auto p : start_to_tracking )
    {
        XLink start_xlink = p.first;
        for( XLink dep_xlink : p.second.deps )
            ASSERT( dep_to_starts.count(dep_xlink) == 1 );            
            
        ASSERT( domain_extension_classes.count(p.second.extra_node)==1 );
        ASSERT( domain_extension_classes.at(p.second.extra_node).count > 0 );
        
        ASSERT( starts_to_redo.count(start_xlink) == 0 ); // should be disjoint
    }
    
    for( auto p : dep_to_starts )
    {
        XLink dep_xlink = p.first;
        for( XLink start_xlink : p.second )
            ASSERT( start_to_tracking.count(start_xlink) == 1 );            
    }
}



void DomainExtensionChannel::InitialBuild()
{
    for( XLink xlink : db->GetDomain().unordered_domain )
        ExtendDomain( xlink );

    Validate();
}


void DomainExtensionChannel::Complete()
{
    // TODO only do what's left over as invalid from previous deletes 
    // and not restored by inserts
#ifdef TRUE_DE
    for( XLink start_xlink : starts_to_redo )
#else
    for( XLink start_xlink : db->GetDomain().unordered_domain )
#endif
        ExtendDomain( start_xlink );
        
    starts_to_redo.clear();   
}


void DomainExtensionChannel::Insert(const DBWalk::WalkInfo &walk_info)
{
#ifdef TRUE_DE
    XLink start_xlink = walk_info.xlink;
    ExtendDomain( start_xlink );
#endif
}


void DomainExtensionChannel::Delete(const DBWalk::WalkInfo &walk_info)
{
#ifdef TRUE_DE
    XLink xlink = walk_info.xlink;
    
    // First deal with the case where the deleted xlink is the start of a domain 
    // extension: in this case, we want to remove every trace of this extension.
    if( start_to_tracking.count(xlink)>0 )
    {
        DropStartXlink( xlink );
        // Don't add to redo: start was deleted, we won't want the DE back
    }
    else if( starts_to_redo.count(xlink)>0 )
    {
        // Remove from redo: we won't want the DE back
        EraseSolo(starts_to_redo, xlink);
    }
    
    // Now deal with the case where the deleted xlink is a dependency of a domain 
    // extension: in this case, we want to remove it but remember that we want to 
    // redo those starting xlinks at Complete() time.
    if( dep_to_starts.count(xlink)>0 )
    {
        set<XLink> start_xlinks = dep_to_starts.at(xlink);
        for( XLink start_xlink : start_xlinks )
        {
            ASSERT( start_to_tracking.count(start_xlink)>0 );
            
            DropStartXlink( start_xlink );
            InsertSolo(starts_to_redo, start_xlink);
        }
    }    
#endif    
}


void DomainExtensionChannel::InsertExtra(const DBWalk::WalkInfo &walk_info)
{
}


void DomainExtensionChannel::DeleteExtra(const DBWalk::WalkInfo &walk_info)
{
}


DomainExtensionChannel::ExtClass::ExtClass( XLink extra_xlink_, int count_ ) :
    extra_xlink( extra_xlink_ ),
    count( count_ )
{
}


string DomainExtensionChannel::ExtClass::GetTrace() const 
{ 
    return "(extra_xlink="+Trace(extra_xlink)+SSPrintf(", count=%d)", count); 
}


DomainExtensionChannel::TrackingRow::TrackingRow( TreePtr<Node> extra_node_, set<XLink> deps_ ) :
    extra_node( extra_node_ ),
    deps( move(deps_) )
{
}


string DomainExtensionChannel::TrackingRow::GetTrace() const 
{ 
    return "(extra_node="+Trace(extra_node)+", deps=%"+Trace(deps)+")";
}
