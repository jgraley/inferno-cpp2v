#include "x_tree_database.hpp"

#include "lacing.hpp"

#include "common/read_args.hpp"
#include "tree_zone.hpp"
#include "free_zone.hpp"

using namespace SR;    

// We won't normally expect matches as postconditions to our
// public methods because changing strategies make some do more
// and others do less. But:
// - FullX methods should erase history, so can always check
// - All methods should match if strategy does not change, as
//   when testing the test
//#define DB_TEST_THE_TEST

XTreeDatabase::XTreeDatabase( TreePtr<Node> main_root, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders ) :
    domain( make_shared<Domain>() ),
    link_table( make_shared<LinkTable>() ),
    node_table( make_shared<NodeTable>() ),
    orderings( make_shared<Orderings>(lacing, this) ),
    domain_extension( make_shared<DomainExtension>(this, domain_extenders) )
{
	auto sp_main_root = make_shared<TreePtr<Node>>(main_root);
    XLink main_root_xlink = XLink::CreateFrom(sp_main_root);    

    ASSERT( main_root_xlink );
    trees_by_ordinal[DBCommon::TreeOrdinal::MAIN] = {sp_main_root};
	next_tree_ordinal = DBCommon::TreeOrdinal::EXTRAS;

    auto create_extra_tree = [=](TreePtr<Node> root_node) -> DBCommon::TreeOrdinal
    {   
		DBCommon::TreeOrdinal tree_ordinal = AllocateExtraTree(root_node);
        de_extra_insert_queue.push( tree_ordinal );       
        return tree_ordinal;
    };

    auto destroy_extra_tree = [=](DBCommon::TreeOrdinal tree_ordinal)
	{
        extra_tree_destroy_queue.push(tree_ordinal);
    };
    
    domain_extension->SetOnExtraTreeFunctions( create_extra_tree, 
                                               destroy_extra_tree );
}

    
DBCommon::TreeOrdinal XTreeDatabase::AllocateExtraTree(TreePtr<Node> root_node)
{
	auto sp_root = make_shared<TreePtr<Node>>(root_node);
	XLink root_xlink = XLink::CreateFrom(sp_root);    
	DBCommon::TreeOrdinal assigned_ordinal;
	if( free_tree_ordinals.empty() )
	{
		assigned_ordinal = next_tree_ordinal;
		((int &)next_tree_ordinal)++;
	}
	else
	{
		assigned_ordinal = free_tree_ordinals.front();
		free_tree_ordinals.pop();
	}
	trees_by_ordinal[assigned_ordinal] = {sp_root};
	return assigned_ordinal;
}

	
void XTreeDatabase::FreeExtraTree(DBCommon::TreeOrdinal tree_ordinal)
{
	trees_by_ordinal.erase(tree_ordinal);
	free_tree_ordinals.push(tree_ordinal);
}


XLink XTreeDatabase::GetRootXLink(DBCommon::TreeOrdinal tree_ordinal) const
{
	return XLink::CreateFrom( trees_by_ordinal.at(tree_ordinal).sp_tp_root_node );
}


void XTreeDatabase::InitialBuild()
{      
    INDENT("p");
	
    DBWalk::Actions actions;
    actions.push_back( domain->GetInsertAction() );
    actions.push_back( link_table->GetInsertAction() );
    actions.push_back( node_table->GetInsertAction() );
    actions.push_back( orderings->GetInsertAction() );
    	
	for( auto p : trees_by_ordinal )
		db_walker.WalkTree( &actions, GetRootXLink(p.first), p.first, DBWalk::WIND_IN );

    domain_extension->InitialBuild();
    
    while(!de_extra_insert_queue.empty())
    {
		ExtraTreeInsert( de_extra_insert_queue.front() );
		de_extra_insert_queue.pop();
	}    
}


void XTreeDatabase::MainTreeReplace( TreeZone target_tree_zone, FreeZone source_free_zone )
{
	ASSERT( target_tree_zone.GetNumTerminii() == source_free_zone.GetNumTerminii() );	
	target_tree_zone.DBCheck(this); // Move back to MainTreeReplace once this is empty
	MutableTreeZone mutable_target_tree_zone( target_tree_zone, GetMutator(target_tree_zone.GetBaseXLink()) );
	
	// Store the core info for the base locally since the link table will change
	// as this function executes.
	const DBCommon::CoreInfo base_info = link_table->GetCoreInfo( target_tree_zone.GetBaseXLink() );

    // Update database 
    MainTreeDelete( mutable_target_tree_zone, &base_info );   
    
    // Patch the tree
    mutable_target_tree_zone.Patch( move(source_free_zone) ); 

    // Update database 
    MainTreeInsert( mutable_target_tree_zone, &base_info );   	
    
    // Update domain extnsion extra trees
    PerformQueuedExtraTreeActions();
}


void XTreeDatabase::MainTreeDelete(TreeZone zone, const DBCommon::CoreInfo *base_info)
{
    INDENT("d");

	ASSERT( extra_tree_destroy_queue.empty() );
	
    DBWalk::Actions actions;
	actions.push_back( domain_extension->GetDeleteAction() );
	actions.push_back( orderings->GetDeleteAction() );
    db_walker.WalkZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   

	DBWalk::Actions actions2;
	actions2.push_back( node_table->GetDeleteAction() );
	actions2.push_back( link_table->GetDeleteAction() );
	actions2.push_back( domain->GetDeleteAction() );
    db_walker.WalkZone( &actions2, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   
}


void XTreeDatabase::MainTreeInsert(TreeZone zone, const DBCommon::CoreInfo *base_info)
{
    INDENT("i");
	ASSERT( de_extra_insert_queue.empty() );

    DBWalk::Actions actions;
	actions.push_back( domain->GetInsertAction() );
	actions.push_back( link_table->GetInsertAction() );
	actions.push_back( node_table->GetInsertAction() );
    db_walker.WalkZone( &actions, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );

	DBWalk::Actions actions3;
	actions3.push_back( orderings->GetInsertAction() );
    db_walker.WalkZone( &actions3, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );
    
	// Domain extension wants to roam around the XTree, consulting
	// parents, children, anything really. So we need a separate pass.
	DBWalk::Actions actions2;
	actions2.push_back( domain_extension->GetInsertAction());
	db_walker.WalkZone( &actions2, zone, DBCommon::TreeOrdinal::MAIN, DBWalk::WIND_IN, base_info );
}


void XTreeDatabase::PerformQueuedExtraTreeActions()
{
	while(!extra_tree_destroy_queue.empty())
	{
		ExtraTreeDelete( extra_tree_destroy_queue.front() );
		FreeExtraTree( extra_tree_destroy_queue.front() );
		
		extra_tree_destroy_queue.pop();
	}
	while(!de_extra_insert_queue.empty())
	{
		ExtraTreeInsert( de_extra_insert_queue.front() );
		
		de_extra_insert_queue.pop();
	}
}


void XTreeDatabase::ExtraTreeDelete(DBCommon::TreeOrdinal tree_ordinal)
{		
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal::EXTRAS );
    XLink xlink = GetRootXLink(tree_ordinal);

    // Note not symmetrical with InsertExtra(): we
    // will be invoked with every xlink in the extra
    // zones and on each call we delete just that
    // xlink.
    DBWalk::Actions actions;
	actions.push_back( domain_extension->GetDeleteAction() );
	actions.push_back( orderings->GetDeleteAction() );
	actions.push_back( node_table->GetDeleteAction() );
	actions.push_back( link_table->GetDeleteAction() );
	actions.push_back( domain->GetDeleteAction() );
    db_walker.WalkTree( &actions, xlink, tree_ordinal, DBWalk::WIND_OUT );       
}


void XTreeDatabase::ExtraTreeInsert(DBCommon::TreeOrdinal tree_ordinal)
{		
    ASSERT( tree_ordinal >= DBCommon::TreeOrdinal::EXTRAS );
    XLink xlink = GetRootXLink(tree_ordinal);
    
	DBWalk::Actions actions;
    actions.push_back( domain->GetInsertAction() );
    actions.push_back( link_table->GetInsertAction() );
    actions.push_back( node_table->GetInsertAction() );
    actions.push_back( orderings->GetInsertAction() );
	db_walker.WalkTree( &actions, xlink, tree_ordinal, DBWalk::WIND_IN );

	DBWalk::Actions actions2;
	actions2.push_back( domain_extension->GetInsertAction());
	db_walker.WalkTree( &actions2, xlink, tree_ordinal, DBWalk::WIND_IN );
}


const DomainExtensionChannel *XTreeDatabase::GetDEChannel( const DomainExtension::Extender *extender ) const
{
	return domain_extension->GetChannel(extender);
}


void XTreeDatabase::PostUpdateActions()
{
	domain_extension->PostUpdateActions();
	
    while(!de_extra_insert_queue.empty())
    {
		ExtraTreeInsert( de_extra_insert_queue.front() );
		de_extra_insert_queue.pop();
	}	
}

	
const Domain &XTreeDatabase::GetDomain() const
{
	return *domain;
}


const LinkTable &XTreeDatabase::GetLinkTable() const
{
	return *link_table;
}


const LinkTable::Row &XTreeDatabase::GetRow(XLink xlink) const
{
	return link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
	return link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
	return node_table->GetRow(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
	return node_table->HasRow(node);
}


XLink XTreeDatabase::TryGetParentXLink(XLink xlink) const
{
	TreePtr<Node> parent_node = GetRow(xlink).parent_node;
	if( !parent_node )
		return XLink();
		
	const set<XLink> &ps = GetNodeRow(parent_node).incoming_xlinks;

    // Note that the parent is unique because:
    // - row is relative to a link, not a node,
    // - multiple parents only allowed at leaf (see #217), and parent is 
    //   (at least) one level back from that.
	switch( ps.size() )
	{
		case 0: return XLink(); 
		case 1: return OnlyElementOf(ps);
		default: ASSERTFAIL("Rule #217 violation: node with child TreePtr has multiple parents");
	}		
}


XLink XTreeDatabase::GetXLink( const TreePtrInterface *px ) const
{
	XLink xlink = TryGetXLink(px);
	ASSERT( xlink )("GetXLink() could not find link for ")(px);
	return xlink;
}


XLink XTreeDatabase::TryGetXLink( const TreePtrInterface *px ) const
{
	TreePtr<Node> child_node = (TreePtr<Node>)*px;

	if( !HasNodeRow(child_node) )
		return XLink(); // fail
	
    NodeTable::Row row = GetNodeRow(child_node);

    for( XLink xlink : row.incoming_xlinks )
		if( xlink.GetTreePtrInterface() == px )
			return xlink;
	
    return XLink(); // fail
}


XLink XTreeDatabase::GetLastDescendant(XLink xlink) const
{
    TreePtr<Node> x = xlink.GetChildTreePtr();
    ASSERT(x)("This probably means we're walking an incomplete tree");
    vector< Itemiser::Element * > x_items = x->Itemise();

	// Loop backward over the items
    for( int item_ordinal=x_items.size()-1; item_ordinal>=0; item_ordinal-- )
    {
        Itemiser::Element *xe = x_items[item_ordinal];
		if( auto x_con = dynamic_cast<ContainerInterface *>(xe) )
		{
			if( !x_con->empty() )
				return GetLastDescendant( XLink( x, &x_con->back() ) );
		}
		else if( auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
		{
			if( *p_x_singular ) // tolerate NULL singlar child pointers
        		return GetLastDescendant( XLink( x, &*p_x_singular ) );
		}
		else
			ASSERTFAIL("got something strange from itemise");
	}

	// No children so we are our our own last descendant
	return xlink;	
}


const Orderings &XTreeDatabase::GetOrderings() const
{
	return *orderings;
}


TreePtr<Node> XTreeDatabase::GetMainRootNode() const
{
	return GetMainRootXLink().GetChildTreePtr();
}


XLink XTreeDatabase::GetMainRootXLink() const
{
	return GetRootXLink(DBCommon::TreeOrdinal::MAIN);
}


unique_ptr<Mutator> XTreeDatabase::GetMutator(XLink xlink)
{
	const LinkTable::Row &row = link_table->GetRow(xlink);
	
	switch( row.context_type )
	{
		case DBCommon::ROOT:
		{
			// This shared_ptr points to the same TreePtr<> instance referenced by the root XLink.
			// By holding it separately, we can avoid a const cast, and in fact constness propagates
			// correctly from the XTreeDatabase object, which is why this method cannot be const.
			ASSERT( (int)(row.tree_ordinal) >= 0 ); // Should be valid whenever context is ROOT
			shared_ptr<TreePtr<Node>> sp_tp_root_node = trees_by_ordinal[row.tree_ordinal].sp_tp_root_node;
			return make_unique<SingularMutator>( row.parent_node, sp_tp_root_node.get() );
		}	
		case DBCommon::SINGULAR:
		{
			vector< Itemiser::Element * > x_items = row.parent_node->Itemise();
			Itemiser::Element *xe = x_items[row.item_ordinal];		
			auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe);
			ASSERT( p_x_singular );
			return make_unique<SingularMutator>( row.parent_node, p_x_singular );
		}
		case DBCommon::IN_SEQUENCE:
		case DBCommon::IN_COLLECTION: 
		{
			// COLLECTION is the motivating case: its elements are const, so we neet Mutate() to change them
			return make_unique<ContainerMutator>( row.parent_node, row.p_container, row.container_it );			
		}
	}	
	ASSERTFAIL();
}


void XTreeDatabase::Dump() const
{
    orderings->Dump();
}


void XTreeDatabase::TestRelations()
{
    if( ReadArgs::test_rel )
    {
		domain_extension->TestRelations( domain->unordered_domain );
		orderings->TestRelations( domain->unordered_domain );
	}		
}
