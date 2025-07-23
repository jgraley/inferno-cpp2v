#include "domain_extension.hpp"

#include "relation_test.hpp"
#include "x_tree_database.hpp"

#include "../agents/agent.hpp"
#include "../agents/relocating_agent.hpp"
#include "helpers/simple_duplicate.hpp"

#define NO_ACTION_ON_SCAFFOLD
#define LEAK_EXTRA_TREES

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


DomainExtension::DomainExtension( XTreeDatabase *db, ExtenderSet extenders )
{
    for( const Extender *extender : extenders )
         channels[extender] = make_unique<DomainExtensionChannel>(db, extender);
}


const DomainExtensionChannel *DomainExtension::GetChannel( const Extender *extender ) const
{
    ASSERT( channels.count(extender) );
    return channels.at(extender).get();
}


void DomainExtension::PerformDeferredActions()
{
    for( auto &p : channels )
        p.second->PerformDeferredActions();
}


void DomainExtension::PerformDeeplyDeferredActions()
{
    for( auto &p : channels )
        p.second->PerformDeeplyDeferredActions();
}


void DomainExtension::InsertTree(const TreeZone &zone)
{     
    auto action = [&](const DBWalk::WalkInfo &walk_info)
    {
	 	InsertAction( walk_info.xlink );
	};	
	db_walker.WalkTreeZone( action, zone, DBWalk::WIND_IN );
}


void DomainExtension::DeleteTree(const TreeZone &zone)
{
    auto action = [&](const DBWalk::WalkInfo &walk_info)
    {
	 	DeleteAction( walk_info.xlink );
	};
	db_walker.WalkTreeZone( action, zone, DBWalk::WIND_OUT );
}


DomainExtension::SwapTransaction::SwapTransaction(DomainExtension *domain_extension_, TreeZone &zone1_, TreeZone &zone2_ ) :
	DBCommon::SwapTransaction( zone1_, zone2_ ),
	domain_extension( *domain_extension_ )
{	
	domain_extension.DeleteTree(zone1);
	domain_extension.DeleteTree(zone2);
}


DomainExtension::SwapTransaction::~SwapTransaction()
{
	domain_extension.InsertTree(zone1);
	domain_extension.InsertTree(zone2);
}


void DomainExtension::InsertAction(XLink xlink)
{        
#ifdef NO_ACTION_ON_SCAFFOLD	
	if( dynamic_cast<ScaffoldBase *>(xlink.GetChildTreePtr().get()) )
		return;
#endif
		
    for( auto &p : channels )
         p.second->InsertAction( xlink );             
}


void DomainExtension::DeleteAction(XLink xlink)
{        
#ifdef NO_ACTION_ON_SCAFFOLD	
	if( dynamic_cast<ScaffoldBase *>(xlink.GetChildTreePtr().get()) )
		return;
#endif

    for( auto &p : channels )
         p.second->DeleteAction( xlink );
};


void DomainExtension::Validate() const
{
    for( auto &p : channels )
        p.second->Validate();
}

// ------------------------- DomainExtensionChannel --------------------------

DomainExtensionChannel::DomainExtensionChannel( XTreeDatabase *db_, const DomainExtension::Extender *extender_ ) :
    db( db_ ),
    extender( extender_ )
{
}


void DomainExtensionChannel::InsertAction(XLink xlink)
{    
	//Validate();    
    // We're now deferring all new stimulus checks because tree update can
    // confuse transformations by eg having two declarers in two separate trees.
    // Wait until we're done with all that stuff then do the checks.
    InsertSolo(stimulii_to_recheck, xlink);
	//Validate();
}


void DomainExtensionChannel::DeleteAction(XLink xlink)
{
    INDENT("D");
	//Validate();

    // First deal with the case where the deleted xlink is the stimulus for a domain 
    // extension: in this case, we want to remove every trace of this extension.
    if( stimulus_to_induced_root_and_deps.count(xlink)>0 )
    {
        TRACE("Extender ")(extender)(": drop stimulus: ")(xlink)("\n");
        DropStimulusXLink( xlink );
        // Don't add to recheck set: stimulus was deleted, we won't want the DE back
    }
    else if( stimulii_to_recheck.count(xlink)>0 )
    {
        // Remove from recheck set: we won't want the DE back
        EraseSolo(stimulii_to_recheck, xlink);
    }
        
	//Validate();    
    // Now deal with the case where the deleted xlink is a dependency of a domain 
    // extension: in this case, we want to remove it but remember that we want to 
    // recheck the stimulus xlink after updates are done.
    if( dep_to_all_stimulii.count(xlink)>0 )
    {
        const set<XLink> stimulus_xlinks = dep_to_all_stimulii.at(xlink);
        for( XLink stimulus_xlink : stimulus_xlinks )
        {
            ASSERT( stimulus_to_induced_root_and_deps.count(stimulus_xlink)>0 );
            
            TRACE("Extender ")(extender)(": drop stimulus: ")(stimulus_xlink)(" via dep: ")(xlink)("\n");
            DropStimulusXLink( stimulus_xlink );
            InsertSolo(stimulii_to_recheck, stimulus_xlink);
        }
    }    
	//Validate();    
}


XLink DomainExtensionChannel::GetUniqueDomainExtension( XLink stimulus_xlink, TreePtr<Node> generated_root ) const
{   
    ASSERT( generated_root );
    ASSERT( induced_root_to_tree_ordinal_and_ref_count.count(generated_root) > 0 )
          ("Generated root ")(generated_root)(" not found in induced_root_to_tree_ordinal_and_ref_count:\n")
          (induced_root_to_tree_ordinal_and_ref_count);
    
    // Cross-checks using stimulus_xlink (rather than acting as a cache, 
    // which we can now do, see #700)
    ASSERT( stimulus_xlink );
    ASSERT( stimulus_to_induced_root_and_deps.count(stimulus_xlink)>0 );
    TreePtr<Node> induced_root = stimulus_to_induced_root_and_deps.at(stimulus_xlink).induced_root;    
    SimpleCompare sc;
    ASSERT( sc.Compare3Way(generated_root, induced_root)==0 ); 

	//Validate();

    // Actual uniquify is just a lookup in the map
    return db->GetRootXLink( induced_root_to_tree_ordinal_and_ref_count.at( generated_root ).tree_ordinal );
}


void DomainExtensionChannel::CreateExtraTree( TreePtr<Node> induced_root )
{
	INDENT("C");
    ASSERT( induced_root );
  
    // To ensure compliance with rule #217 we must duplicate the tree that
    // we were given, in case it meanders into the main X tree not at an
    // identifier, causing illegal multiple parents. See #677
    // TODO maybe only do this if subtree actually would go wrong.
    TreePtr<Node> extra_root_node = SimpleDuplicate::DuplicateSubtree( induced_root );
	// TODO move up and wrap "Duplicate" in FreeZone class
	auto extra_free_zone = FreeZone::CreateSubtree(extra_root_node);    

    // Add this xlink and ordinal to the extension classes as stimulus. 
    // Count begins at 1 since there's one ref (this one)
    induced_root_to_tree_ordinal_and_ref_count.insert( make_pair( extra_root_node, ExtensionClass() ) );      
	//Validate();

    // Add the whole subtree to the rest of the database as a new tree
    // This can re-enter our code, so ensure Validate() passes before calling
    DBCommon::TreeOrdinal tree_ordinal = db->BuildTree( DBCommon::TreeType::DOMAIN_EXTENSION, extra_free_zone );;        

    induced_root_to_tree_ordinal_and_ref_count.at(extra_root_node).tree_ordinal = tree_ordinal;      
}


void DomainExtensionChannel::CheckStimulusXLink( XLink stimulus_xlink )
{
	INDENT("H");
    DomainExtension::Extender::Info info = extender->GetDomainExtension( db, stimulus_xlink );  
	//Validate();    
    if( !info.induced_base_node )
        return;
        
    //ASSERT( deps.size() < 20 )("Big deps for ")(stimulus_xlink)("\n")(deps);
    
    //FTRACE("Stimulus XLink: ")(stimulus_xlink)
    //      (" induces: ")(info.induced_base_node)
    //      (" with deps: ")(info.deps)("\n");

    for( XLink dep : info.deps )
        dep_to_all_stimulii[dep].insert(stimulus_xlink); 
    stimulus_to_induced_root_and_deps.insert( make_pair( stimulus_xlink, InducedAndDeps(info.induced_base_node, info.deps) ) ); 
    
    // If there's already a class for this node, return it and early-out
    // Note: this is done by simple compare, and identity is not 
    // required. This makes for a very "powerful" search for existing
    // candidates.
    // TODO try SC search over entire DB. Early-out if in main tree. Otherwise
    // continue, but induced_root_to_tree_ordinal_and_ref_count is now keyed by
    // tree id (even if the hit was not at base). This provides the required keep-alive effect.
    if( induced_root_to_tree_ordinal_and_ref_count.count(info.induced_base_node) > 0 )
    {
        induced_root_to_tree_ordinal_and_ref_count.at(info.induced_base_node).ref_count++;
    }
	else
	{
		// An extra tree is required
		CreateExtraTree( info.induced_base_node );
	}
	
	// Fix the induced_root in the stimulus_to_induced_root_and_deps to be the duplicate (which
	// will be the key in induced_root_to_tree_ordinal_and_ref_count due to the action of
	// CreateExtraTree()). This is required because the original induced tree could change due 
	// to future replace actions, whereas the duplicate won't. See #785
	TreePtr<Node> induced_root = induced_root_to_tree_ordinal_and_ref_count.find(info.induced_base_node)->first;
	stimulus_to_induced_root_and_deps.at(stimulus_xlink).induced_root = induced_root;
	
	//Validate();    
	//FTRACE("induced_root_to_tree_ordinal_and_ref_count: ")(induced_root_to_tree_ordinal_and_ref_count)("\n");
}
#include "../tree/cpptree.hpp"

void DomainExtensionChannel::DropStimulusXLink( XLink stimulus_xlink )
{
	INDENT("R");
    TRACE("Stimulus: ")(stimulus_xlink)("\n");
    // Be strict here: all these data structures need to remain in synch
    ASSERT( stimulus_to_induced_root_and_deps.count(stimulus_xlink)>0 );
   
	//Validate();    
 
	// Note: most robust is to delete immediately (aside from the teardown) but defer the 
	// check/create to the very end of tree update. Asymmetry approved!
	// Warning can causae re-entry: at the very least, ensure Validate() passes before calling

	TreePtr<Node> induced_root = stimulus_to_induced_root_and_deps.at(stimulus_xlink).induced_root;
	set<XLink> &deps = stimulus_to_induced_root_and_deps.at(stimulus_xlink).deps;

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
	
	// Remove tracking row for this stimulus xlink
	EraseSolo(stimulus_to_induced_root_and_deps, stimulus_xlink);

	//Validate();    

	// Remove a reference to this induced root from domain extension classes, possibly
	// dropping the extension class completely.
	ASSERT( induced_root_to_tree_ordinal_and_ref_count.count(induced_root) > 0 )
		  ("Induced root ")(induced_root)(" was expected to be in ")(induced_root_to_tree_ordinal_and_ref_count);
	int new_rc = --induced_root_to_tree_ordinal_and_ref_count.at(induced_root).ref_count;    
	if( new_rc > 0 )
		return;
	
	//FTRACE("Dropping induced ")(induced_root)("\n");
	DBCommon::TreeOrdinal ord = induced_root_to_tree_ordinal_and_ref_count.at(induced_root).tree_ordinal;
	EraseSolo( induced_root_to_tree_ordinal_and_ref_count, induced_root );
	
	ordinals_to_tear_down.insert( ord );     
    
	//Validate();    
}


void DomainExtensionChannel::PerformDeferredActions()
{
    // TODO only do what's left over as invalid from previous deletes 
    // and not restored by inserts
	//Validate();    
    for( XLink stimulus_xlink : stimulii_to_recheck )
        CheckStimulusXLink( stimulus_xlink );
        
    stimulii_to_recheck.clear();   

    Validate();
}


void DomainExtensionChannel::PerformDeeplyDeferredActions()
{
	// Wait until all embedded engines are complete (i.e. end of step)
	// because couplings may reference into domain extension trees which
	// have been determined as unnecessary for future searches (because 
	// there's no stimulus for them).
	for( DBCommon::TreeOrdinal ord : ordinals_to_tear_down )		
		db->TeardownTree(ord); 
	ordinals_to_tear_down.clear();
}


void DomainExtensionChannel::Validate() const
{
    ASSERT( stimulus_to_induced_root_and_deps.size() <= db->GetDomain().unordered_domain.size() );
    
    for( auto p : stimulus_to_induced_root_and_deps )
    {
        XLink stimulus_xlink = p.first;
        for( XLink dep : p.second.deps )
            ASSERT( dep_to_all_stimulii.count(dep) != 0 )
                  (dep)
                  (": domain %d: %d stimulii, %d deps:", db->GetDomain().unordered_domain.size(), stimulus_to_induced_root_and_deps.size(), dep_to_all_stimulii.size())
                  (dep_to_all_stimulii);            
        
        auto induced_root = p.second.induced_root;
        ASSERT( induced_root_to_tree_ordinal_and_ref_count.count(induced_root)==1 )
              (induced_root)
              (": domain %d: %d stimulii, %d deps\n", db->GetDomain().unordered_domain.size(), stimulus_to_induced_root_and_deps.size(), dep_to_all_stimulii.size())
              (induced_root_to_tree_ordinal_and_ref_count);
        ASSERT( induced_root_to_tree_ordinal_and_ref_count.at(induced_root).ref_count > 0 )
              (induced_root)
              (": domain %d: %d stimulii, %d deps\n", db->GetDomain().unordered_domain.size(), stimulus_to_induced_root_and_deps.size(), dep_to_all_stimulii.size())
              (induced_root_to_tree_ordinal_and_ref_count);
    }
    
    for( auto p : dep_to_all_stimulii )
    {
        for( XLink stimulus_xlink : p.second )
            ASSERT( stimulus_to_induced_root_and_deps.count(stimulus_xlink) == 1 )
                  (stimulus_xlink)
                  (": domain %d: %d stimulii, %d deps\n", db->GetDomain().unordered_domain.size(), stimulus_to_induced_root_and_deps.size(), dep_to_all_stimulii.size());            
    }    
}


string DomainExtensionChannel::GetTrace() const 
{
	return SSPrintf("DomainExtensionChannel@%p for ", this)+extender->GetTrace();
}



DomainExtensionChannel::ExtensionClass::ExtensionClass() :
    ref_count( 1 )
{
}


string DomainExtensionChannel::ExtensionClass::GetTrace() const 
{ 
    return "(tree_ordinal="+Trace((int)tree_ordinal)+SSPrintf(", ref_count=%d)", ref_count); 
}


DomainExtensionChannel::InducedAndDeps::InducedAndDeps( TreePtr<Node> induced_root_, set<XLink> deps_ ) :
    induced_root( induced_root_ ),
    deps( deps_ )
{
}


string DomainExtensionChannel::InducedAndDeps::GetTrace() const 
{ 
    return "(induced_base_node="+Trace(induced_root)+", deps=%"+Trace(deps)+")";
}
