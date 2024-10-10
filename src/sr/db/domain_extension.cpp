#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "../agents/teleport_agent.hpp"
#include "helpers/simple_duplicate.hpp"

//#define TRACE_DOMAIN_EXTEND

#define TRUE_DE

using namespace SR;    

// ------------------------- DomainExtension --------------------------

bool DomainExtension::ExtenderChannelRelation::operator()( const Extender *l, const Extender *r ) const
{
	return l->IsExtenderChannelLess( *r );
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
	

void DomainExtension::SetOnExtraTreeFunctions( OnExtraTreeFunction on_insert_extra_zone_,
                                                OnExtraTreeFunction on_delete_extra_zone_ )
{
	for( auto &p : channels )
		p.second->SetOnExtraTreeFunctions( on_insert_extra_zone_, on_delete_extra_zone_ );
}


const DomainExtensionChannel *DomainExtension::GetChannel( const Extender *extender ) const
{
    ASSERT( channels.count(extender) );
    return channels.at(extender).get();
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


void DomainExtensionChannel::SetOnExtraTreeFunctions( DomainExtension::OnExtraTreeFunction on_insert_extra_tree_,
                                                       DomainExtension::OnExtraTreeFunction on_delete_extra_tree_ )
{
    on_insert_extra_tree = on_insert_extra_tree_;
    on_delete_extra_tree = on_delete_extra_tree_;
}


XLink DomainExtensionChannel::GetUniqueDomainExtension( XLink start_xlink, TreePtr<Node> node ) const
{   
    ASSERT( node );
	ASSERT( extra_root_node_to_xlink_and_refcount.count(node) > 0 )
	      (node)(" not found in extra_root_node_to_xlink_and_refcount:\n")
	      (extra_root_node_to_xlink_and_refcount);
    
    // Cross-checks using start_xlink (rather than acting as a cache, 
    // which we can now do, see #700)
    ASSERT( start_xlink );
    ASSERT( start_to_induced_and_deps.count(start_xlink)>0 );
    TreePtr<Node> induced_base_node = start_to_induced_and_deps.at(start_xlink).induced_base_node;    
    SimpleCompare sc;
    ASSERT( sc.Compare3Way(node, induced_base_node)==0 ); 

    // Actual uniquify is just a lookup in the map
    return extra_root_node_to_xlink_and_refcount.at( node ).induced_base_xlink;
}


void DomainExtensionChannel::AddExtraTree( TreePtr<Node> induced_base_node )
{
    ASSERT( induced_base_node );
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> extra_root_node = SimpleDuplicate::DuplicateSubtree( induced_base_node );
  
    // Create an XLink that will allow us to track this subtree
    XLink extra_root_xlink = XLink::CreateDistinct( extra_root_node );    
   
    // Add this xlink to the extension classes as start. Count starts 
    // at 1 since there's one ref (this one)
	(void)extra_root_node_to_xlink_and_refcount.insert( make_pair( extra_root_node, ExtensionClass(extra_root_xlink, 1) ) );    
        
    // Add the whole subtree to the rest of the database
    on_insert_extra_tree( extra_root_xlink );        
    
    // Ensure the original tree is found in the extension classes now (it wasn't 
    // earlier on) as an extra check
    ASSERT( extra_root_node_to_xlink_and_refcount.count( induced_base_node ) == 1 );
}


void DomainExtensionChannel::TryAddStartXLink( XLink start_xlink )
{
    DomainExtension::Extender::Info info = extender->GetDomainExtension( db, start_xlink );  
    if( !info.induced_base_node )
        return;
    
    //ASSERT( deps.size() < 20 )("Big deps for ")(start_xlink)("\n")(deps);
    
    start_to_induced_and_deps.insert( make_pair( start_xlink, TrackingRow(info.induced_base_node, info.deps) ) ); // TODO do we need this if there's already a class?
    
    for( TreePtr<Node> dep : info.deps )
        dep_to_all_starts[dep].insert(start_xlink); // TODO do we need this if there's already a class?
    
    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    if( extra_root_node_to_xlink_and_refcount.count(info.induced_base_node) > 0 )
    {
        extra_root_node_to_xlink_and_refcount.at(info.induced_base_node).ref_count++;
        return; 
    }
        
    // An extra tree is required
    AddExtraTree( info.induced_base_node );
}


void DomainExtensionChannel::DropStartXlink( XLink start_xlink )
{
    // Be strict here: all these data structures need to remain in synch
    ASSERT( start_to_induced_and_deps.count(start_xlink)>0 );
    
    TreePtr<Node> induced_base_node = start_to_induced_and_deps.at(start_xlink).induced_base_node;
    set<TreePtr<Node>> &deps = start_to_induced_and_deps.at(start_xlink).deps;

    // Remove this starting xlink from deps structures, possibly dropping the
    // dep completely
    for( TreePtr<Node> dep : deps )
    {
        ASSERT( dep_to_all_starts.count(dep)>0 );
        ASSERT( !dep_to_all_starts.at(dep).empty() );
        EraseSolo(dep_to_all_starts.at(dep), start_xlink);
        if( dep_to_all_starts.at(dep).empty() )
            EraseSolo(dep_to_all_starts, dep);
    }

    // Remove this starting link from domain extension classes, possibly
    // dropping the extension class completely.
    ASSERT( extra_root_node_to_xlink_and_refcount.count(induced_base_node) > 0 );
    int new_rc = --extra_root_node_to_xlink_and_refcount.at(induced_base_node).ref_count;
    if( new_rc==0 )
        EraseSolo( extra_root_node_to_xlink_and_refcount, induced_base_node ); // TODO use on_delete_extra_tree()

    // Remove tracking row for this starting xlink
    EraseSolo(start_to_induced_and_deps, start_xlink);
}


void DomainExtensionChannel::Validate() const
{
	ASSERT( start_to_induced_and_deps.size() <= db->GetDomain().unordered_domain.size() );
	
    for( auto p : start_to_induced_and_deps )
    {
        XLink start_xlink = p.first;
        for( TreePtr<Node> dep : p.second.deps )
            ASSERT( dep_to_all_starts.count(dep) == 1 );            
            
        ASSERT( extra_root_node_to_xlink_and_refcount.count(p.second.induced_base_node)==1 );
        ASSERT( extra_root_node_to_xlink_and_refcount.at(p.second.induced_base_node).ref_count > 0 );
        
        ASSERT( starts_to_redo.count(start_xlink) == 0 ); // should be disjoint
    }
    
    for( auto p : dep_to_all_starts )
    {
        for( XLink start_xlink : p.second )
            ASSERT( start_to_induced_and_deps.count(start_xlink) == 1 );            
    }
    
	//FTRACE(extender)(" domain %d: %d starts, %d deps\n", db->GetDomain().unordered_domain.size(), start_to_induced_and_deps.size(), dep_to_all_starts.size());    
}



void DomainExtensionChannel::InitialBuild()
{
    for( XLink xlink : db->GetDomain().unordered_domain )
        TryAddStartXLink( xlink );
	
    Validate();
}


void DomainExtensionChannel::Complete()
{
    // TODO only do what's left over as invalid from previous deletes 
    // and not restored by inserts
    for( XLink start_xlink : starts_to_redo )
        TryAddStartXLink( start_xlink );
        
    starts_to_redo.clear();   

    Validate();
}


void DomainExtensionChannel::Insert(const DBWalk::WalkInfo &walk_info)
{
    XLink start_xlink = walk_info.xlink;
    TryAddStartXLink( start_xlink );
}


void DomainExtensionChannel::Delete(const DBWalk::WalkInfo &walk_info)
{
    XLink xlink = walk_info.xlink;
    
    // First deal with the case where the deleted xlink is the start of a domain 
    // extension: in this case, we want to remove every trace of this extension.
    if( start_to_induced_and_deps.count(xlink)>0 )
    {
        DropStartXlink( xlink );
        // Don't add to redo: start was deleted, we won't want the DE back
    }
    else if( starts_to_redo.count(xlink)>0 )
    {
        // Remove from redo: we won't want the DE back
        EraseSolo(starts_to_redo, xlink);
    }
    
    TreePtr<Node> x = walk_info.x;
    
    // Now deal with the case where the deleted xlink is a dependency of a domain 
    // extension: in this case, we want to remove it but remember that we want to 
    // redo the starting xlink at Complete() time.
    if( dep_to_all_starts.count(x)>0 )
    {
        set<XLink> start_xlinks = dep_to_all_starts.at(x);
        for( XLink start_xlink : start_xlinks )
        {
            ASSERT( start_to_induced_and_deps.count(start_xlink)>0 );
            
            DropStartXlink( start_xlink );
            InsertSolo(starts_to_redo, start_xlink);
        }
    }    
}


void DomainExtensionChannel::InsertExtra(const DBWalk::WalkInfo &walk_info)
{
	// This is where we get to as a result of having added an extra tree.
	// We could behave like Insert() and see if the extra tree induces
	// anything more.
}


void DomainExtensionChannel::DeleteExtra(const DBWalk::WalkInfo &walk_info)
{
}


DomainExtensionChannel::ExtensionClass::ExtensionClass( XLink induced_base_xlink_, int ref_count_ ) :
    induced_base_xlink( induced_base_xlink_ ),
    ref_count( ref_count_ )
{
}


string DomainExtensionChannel::ExtensionClass::GetTrace() const 
{ 
    return "(induced_base_xlink="+Trace(induced_base_xlink)+SSPrintf(", ref_count=%d)", ref_count); 
}


DomainExtensionChannel::TrackingRow::TrackingRow( TreePtr<Node> induced_base_node_, set<TreePtr<Node>> deps_ ) :
    induced_base_node( induced_base_node_ ),
    deps( deps_ )
{
}


string DomainExtensionChannel::TrackingRow::GetTrace() const 
{ 
    return "(induced_base_node="+Trace(induced_base_node)+", deps=%"+Trace(deps)+")";
}
