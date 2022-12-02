#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "../agents/teleport_agent.hpp"
#include "helpers/duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

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

    return domain_extension_classes.at( node );
}


void DomainExtensionChannel::ExtendDomainBaseXLink( TreePtr<Node> node )
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
        
    // Add this xlink to the extension classes as initial
	(void)domain_extension_classes.insert( make_pair( extra_xlink.GetChildX(), extra_xlink ) );    
        
    // Add the whole zon to the rest of the database
    on_insert_extra_zone( extra_zone );        
    
    // Ensure the original tree is found in the domain now (it wasn't 
    // earlier on) as an extra check
    ASSERT( domain_extension_classes.count( node ) == 1 );
}


void DomainExtensionChannel::ExtendDomain( const unordered_set<XLink> &new_domain )
{
    for( XLink new_xlink : new_domain )
    {
		set<XLink> deps;
		TreePtr<Node> node = extender->ExpandNormalDomain( db, new_xlink, deps );  
    	if( node )
			ExtendDomainBaseXLink( node );
	}
}


void DomainExtensionChannel::InitialBuild()
{
	ExtendDomain( db->GetDomain().unordered_domain );
}


void DomainExtensionChannel::Complete()
{
    // TODO only do what's left over as invalid from previous deletes 
    // and not restored by inserts
	ExtendDomain( db->GetDomain().unordered_domain );
}


void DomainExtensionChannel::Insert(const DBWalk::WalkInfo &walk_info)
{
}


void DomainExtensionChannel::Delete(const DBWalk::WalkInfo &walk_info)
{
}


void DomainExtensionChannel::InsertExtra(const DBWalk::WalkInfo &walk_info)
{
}


void DomainExtensionChannel::DeleteExtra(const DBWalk::WalkInfo &walk_info)
{
}


