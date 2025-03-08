#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "../agents/relocating_agent.hpp"
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
        if( auto tp_op = dynamic_cast<const RelocatingAgent::RelocateOperator *>(sub_expr) )
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
	

void DomainExtension::SetOnExtraTreeFunctions( CreateExtraTreeFunction on_create_extra_tree_,
                                               DestroyExtraTreeFunction on_destroy_extra_tree_ )
{
	for( auto &p : channels )
		p.second->SetOnExtraTreeFunctions( on_create_extra_tree_, on_destroy_extra_tree_ );
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


void DomainExtension::PostUpdateActions()
{
	for( auto &p : channels )
        p.second->PostUpdateActions();
}


DBWalk::Action DomainExtension::GetInsertAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->Insert( walk_info );			 
	};
}


DBWalk::Action DomainExtension::GetDeleteAction()
{	
	return [=](const DBWalk::WalkInfo &walk_info)
	{        
		for( auto &p : channels )
			 p.second->Delete( walk_info );
	};
}


void DomainExtension::CheckRelations( const unordered_set<XLink> &xlinks )
{	
}

// ------------------------- DomainExtensionChannel --------------------------

DomainExtensionChannel::DomainExtensionChannel( const XTreeDatabase *db_, const DomainExtension::Extender *extender_ ) :
    db( db_ ),
    extender( extender_ )
{
}


void DomainExtensionChannel::SetOnExtraTreeFunctions( DomainExtension::CreateExtraTreeFunction on_create_extra_tree_,
                                                      DomainExtension::DestroyExtraTreeFunction on_destroy_extra_tree_ )
{
    create_extra_tree = on_create_extra_tree_;
    destroy_extra_tree = on_destroy_extra_tree_;
}


XLink DomainExtensionChannel::GetUniqueDomainExtension( XLink stimulus_xlink, TreePtr<Node> node ) const
{   
    ASSERT( node );
	ASSERT( induced_subtree_by_value_to_extra_subtree_and_refcount.count(node) > 0 )
	      (node)(" not found in induced_subtree_by_value_to_extra_subtree_and_refcount:\n")
	      (induced_subtree_by_value_to_extra_subtree_and_refcount);
    
    // Cross-checks using stimulus_xlink (rather than acting as a cache, 
    // which we can now do, see #700)
    ASSERT( stimulus_xlink );
    ASSERT( stimulus_to_induced_and_deps.count(stimulus_xlink)>0 );
    TreePtr<Node> induced_base_node = stimulus_to_induced_and_deps.at(stimulus_xlink).induced_base_node;    
    SimpleCompare sc;
    ASSERT( sc.Compare3Way(node, induced_base_node)==0 ); 

    // Actual uniquify is just a lookup in the map
    return db->GetRootXLink( induced_subtree_by_value_to_extra_subtree_and_refcount.at( node ).tree_ordinal );
}


void DomainExtensionChannel::CreateExtraTree( TreePtr<Node> induced_base_node )
{
    ASSERT( induced_base_node );
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> extra_root_node = SimpleDuplicate::DuplicateSubtree( induced_base_node );
   
    // Add the whole subtree to the rest of the database as a new tree
    DBCommon::TreeOrdinal tree_ordinal = create_extra_tree( extra_root_node );        

    // Add this xlink and ordinal to the extension classes as stimulus. 
    // Count begins at 1 since there's one ref (this one)
	(void)induced_subtree_by_value_to_extra_subtree_and_refcount.insert( make_pair( extra_root_node, ExtensionClass(tree_ordinal, 1) ) );    
}


void DomainExtensionChannel::CheckStimulusXLink( XLink stimulus_xlink )
{
    DomainExtension::Extender::Info info = extender->GetDomainExtension( db, stimulus_xlink );  
    if( !info.induced_base_node )
        return;
        
    //ASSERT( deps.size() < 20 )("Big deps for ")(stimulus_xlink)("\n")(deps);
    
    stimulus_to_induced_and_deps.insert( make_pair( stimulus_xlink, InducedAndDeps(info.induced_base_node, info.deps) ) ); // TODO do we need this if there's already a class?
    
    for( XLink dep : info.deps )
        dep_to_all_stimulii[dep].insert(stimulus_xlink); // TODO do we need this if there's already a class?
    
    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    // TODO try SC search over entire DB. Early-out if in main tree. Otherwise
    // continue, but induced_subtree_by_value_to_extra_subtree_and_refcount is now keyed by
    // tree id (even if the hit was not at base). This provides the required keep-alive effect.
    if( induced_subtree_by_value_to_extra_subtree_and_refcount.count(info.induced_base_node) > 0 )
    {
        induced_subtree_by_value_to_extra_subtree_and_refcount.at(info.induced_base_node).ref_count++;
        return; 
    }

    TRACE("Stimulus XLink ")(stimulus_xlink)(" induces ")(info.induced_base_node)("\n");

        
    // An extra tree is required
    CreateExtraTree( info.induced_base_node );
}


void DomainExtensionChannel::DropStimulusXLink( XLink stimulus_xlink )
{
    // Be strict here: all these data structures need to remain in synch
    ASSERT( stimulus_to_induced_and_deps.count(stimulus_xlink)>0 );
    
    TreePtr<Node> induced_base_node = stimulus_to_induced_and_deps.at(stimulus_xlink).induced_base_node;
    set<XLink> &deps = stimulus_to_induced_and_deps.at(stimulus_xlink).deps;

    // Remove this stimulus xlink from deps structures, possibly dropping the
    // dep completely
    for( XLink dep : deps )
    {
        ASSERT( dep_to_all_stimulii.count(dep)>0 );
        ASSERT( !dep_to_all_stimulii.at(dep).empty() );
        EraseSolo(dep_to_all_stimulii.at(dep), stimulus_xlink);
        if( dep_to_all_stimulii.at(dep).empty() )
            EraseSolo(dep_to_all_stimulii, dep);
    }

    // Remove this stimulus xlink from domain extension classes, possibly
    // dropping the extension class completely.
    ASSERT( induced_subtree_by_value_to_extra_subtree_and_refcount.count(induced_base_node) > 0 );
    int new_rc = --induced_subtree_by_value_to_extra_subtree_and_refcount.at(induced_base_node).ref_count;
    if( new_rc==0 )
    {
		DBCommon::TreeOrdinal tree_ordinal = induced_subtree_by_value_to_extra_subtree_and_refcount.at(induced_base_node).tree_ordinal;
        EraseSolo( induced_subtree_by_value_to_extra_subtree_and_refcount, induced_base_node );
		destroy_extra_tree(tree_ordinal);
	}
	
    // Remove tracking row for this stimulus xlink
    EraseSolo(stimulus_to_induced_and_deps, stimulus_xlink);
}


void DomainExtensionChannel::Validate() const
{
	ASSERT( stimulus_to_induced_and_deps.size() <= db->GetDomain().unordered_domain.size() );
	
    for( auto p : stimulus_to_induced_and_deps )
    {
        XLink stimulus_xlink = p.first;
        for( XLink dep : p.second.deps )
            ASSERT( dep_to_all_stimulii.count(dep) == 1 );            
            
        ASSERT( induced_subtree_by_value_to_extra_subtree_and_refcount.count(p.second.induced_base_node)==1 );
        ASSERT( induced_subtree_by_value_to_extra_subtree_and_refcount.at(p.second.induced_base_node).ref_count > 0 );
        
        ASSERT( stimulii_to_recheck.count(stimulus_xlink) == 0 ); // should be disjoint
    }
    
    for( auto p : dep_to_all_stimulii )
    {
        for( XLink stimulus_xlink : p.second )
            ASSERT( stimulus_to_induced_and_deps.count(stimulus_xlink) == 1 );            
    }
    
	//FTRACE(extender)(" domain %d: %d stimulii, %d deps\n", db->GetDomain().unordered_domain.size(), stimulus_to_induced_and_deps.size(), dep_to_all_stimulii.size());    
}



void DomainExtensionChannel::InitialBuild()
{
	TRACE("Initial DE build for extender ")(*extender)("\n");

    for( XLink xlink : db->GetDomain().unordered_domain )
        CheckStimulusXLink( xlink );
	
    Validate();
}


void DomainExtensionChannel::PostUpdateActions()
{
    // TODO only do what's left over as invalid from previous deletes 
    // and not restored by inserts
    for( XLink stimulus_xlink : stimulii_to_recheck )
        CheckStimulusXLink( stimulus_xlink );
        
    stimulii_to_recheck.clear();   

    Validate();
}


void DomainExtensionChannel::Insert(const DBWalk::WalkInfo &walk_info)
{
    XLink stimulus_xlink = walk_info.xlink;
    CheckStimulusXLink( stimulus_xlink );
}


void DomainExtensionChannel::Delete(const DBWalk::WalkInfo &walk_info)
{
    XLink xlink = walk_info.xlink;
    
    // First deal with the case where the deleted xlink is the stimulus for a domain 
    // extension: in this case, we want to remove every trace of this extension.
    if( stimulus_to_induced_and_deps.count(xlink)>0 )
    {
        DropStimulusXLink( xlink );
        // Don't add to recheck set: stimulus was deleted, we won't want the DE back
    }
    else if( stimulii_to_recheck.count(xlink)>0 )
    {
        // Remove from recheck set: we won't want the DE back
        EraseSolo(stimulii_to_recheck, xlink);
    }
        
    // Now deal with the case where the deleted xlink is a dependency of a domain 
    // extension: in this case, we want to remove it but remember that we want to 
    // recheck the stimulus xlink after updates are done.
    if( dep_to_all_stimulii.count(xlink)>0 )
    {
        const set<XLink> stimulus_xlinks = dep_to_all_stimulii.at(xlink);
        for( XLink stimulus_xlink : stimulus_xlinks )
        {
            ASSERT( stimulus_to_induced_and_deps.count(stimulus_xlink)>0 );
            
            DropStimulusXLink( stimulus_xlink );
            InsertSolo(stimulii_to_recheck, stimulus_xlink);
        }
    }    
}


DomainExtensionChannel::ExtensionClass::ExtensionClass( DBCommon::TreeOrdinal tree_ordinal_, int ref_count_ ) :
    tree_ordinal( tree_ordinal_ ),
    ref_count( ref_count_ )
{
}


string DomainExtensionChannel::ExtensionClass::GetTrace() const 
{ 
    return "(tree_ordinal="+Trace((int)tree_ordinal)+SSPrintf(", ref_count=%d)", ref_count); 
}


DomainExtensionChannel::InducedAndDeps::InducedAndDeps( TreePtr<Node> induced_base_node_, set<XLink> deps_ ) :
    induced_base_node( induced_base_node_ ),
    deps( deps_ )
{
}


string DomainExtensionChannel::InducedAndDeps::GetTrace() const 
{ 
    return "(induced_base_node="+Trace(induced_base_node)+", deps=%"+Trace(deps)+")";
}
