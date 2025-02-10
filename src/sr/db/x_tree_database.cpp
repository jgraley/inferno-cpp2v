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

XTreeDatabase::XTreeDatabase( XLink main_root_xlink_, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders ) :
    domain( make_shared<Domain>() ),
    link_table( make_shared<LinkTable>() ),
    node_table( make_shared<NodeTable>() ),
    orderings( make_shared<Orderings>(lacing, this) ),
    domain_extension( make_shared<DomainExtension>(this, domain_extenders) ),    
    main_root_xlink( main_root_xlink_ )
{
    auto on_insert_extra_tree = [=](XLink extra_root) -> DBCommon::RootOrdinal
    {
		ASSERT( extra_root != main_root_xlink );
		ASSERT( roots_by_xlink.count(extra_root) == 0 );
    
		DBCommon::RootOrdinal assigned_ordinal = next_root_ordinal;
		((int &)next_root_ordinal)++;
		roots_by_xlink[extra_root] = assigned_ordinal;    
		roots_by_id[assigned_ordinal] = extra_root;

        de_extra_insert_queue.push( assigned_ordinal );
        
        return assigned_ordinal;
    };

    auto on_delete_extra_tree = [=](XLink extra_root)
	{
        de_extra_delete_queue.push(extra_root);
        // TODO
        // feed though the ordinals so that we don't need roots_by_xlink and drop it
        // Choose a name!! TreeOrdinal, tree_ordinal
        // Extract functions for: inserting, deleting, access and use from DE so it doesn't hold the root XLink
        // Stop ordinal wrapping from malfunctioning (reuse?)
        // Clarify that the main root XLink in roots_by_id is the true root, owned by this class - VN etc classes etc have to ask for it
        // Audit GetTreePtrInterface()
        // Tix: do we still always need to DuplicateSubtree() in DomainExtensionChannel::ExtraTreeInsert()?
    };
    
    domain_extension->SetOnExtraTreeFunctions( on_insert_extra_tree, 
                                               on_delete_extra_tree );
}

    
void XTreeDatabase::InitialBuild()
{      
    INDENT("p");
    ASSERT( main_root_xlink );
	
    DBWalk::Actions actions;
    actions.push_back( domain->GetInsertAction() );
    actions.push_back( link_table->GetInsertAction() );
    actions.push_back( node_table->GetInsertAction() );
    actions.push_back( orderings->GetInsertAction() );
    
	roots_by_id[DBCommon::RootOrdinal::MAIN] = main_root_xlink;
	roots_by_id[DBCommon::RootOrdinal::MMAX] = XLink::MMAX_Link;
	roots_by_id[DBCommon::RootOrdinal::OFF_END] = XLink::OffEndXLink;
	next_root_ordinal = DBCommon::RootOrdinal::EXTRAS;
	
	for( auto p : roots_by_id )
	{
		roots_by_xlink[p.second] = p.first;
		db_walker.WalkTree( &actions, p.second, p.first, DBWalk::WIND_IN );
	}

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
	MutableTreeZone mutable_target_tree_zone( target_tree_zone, link_table->GetMutator(target_tree_zone.GetBaseXLink()) );
	
	// Store the core info for the base locally since the link table will change
	// as this function executes.
	const DBCommon::CoreInfo base_info = link_table->GetCoreInfo( target_tree_zone.GetBaseXLink() );

    // Update database 
    MainTreeDelete( mutable_target_tree_zone, &base_info );   
    
    // Patch the tree
    mutable_target_tree_zone.Patch( move(source_free_zone) ); 

    // Update database 
    MainTreeInsert( mutable_target_tree_zone, &base_info );   	
}


void XTreeDatabase::MainTreeDelete(TreeZone zone, const DBCommon::CoreInfo *base_info)
{
    INDENT("d");

	ASSERT( de_extra_delete_queue.empty() );
	
    DBWalk::Actions actions;
	actions.push_back( domain_extension->GetDeleteAction() );
	actions.push_back( orderings->GetDeleteAction() );
    db_walker.WalkZone( &actions, zone, DBCommon::RootOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   

	DBWalk::Actions actions2;
	actions2.push_back( node_table->GetDeleteAction() );
	actions2.push_back( link_table->GetDeleteAction() );
	actions2.push_back( domain->GetDeleteAction() );
    db_walker.WalkZone( &actions2, zone, DBCommon::RootOrdinal::MAIN, DBWalk::WIND_OUT, base_info );   
}


void XTreeDatabase::MainTreeInsert(TreeZone zone, const DBCommon::CoreInfo *base_info)
{
    INDENT("i");
	ASSERT( de_extra_insert_queue.empty() );

    DBWalk::Actions actions;
	actions.push_back( domain->GetInsertAction() );
	actions.push_back( link_table->GetInsertAction() );
	actions.push_back( node_table->GetInsertAction() );
    db_walker.WalkZone( &actions, zone, DBCommon::RootOrdinal::MAIN, DBWalk::WIND_IN, base_info );

	DBWalk::Actions actions3;
	actions3.push_back( orderings->GetInsertAction() );
    db_walker.WalkZone( &actions3, zone, DBCommon::RootOrdinal::MAIN, DBWalk::WIND_IN, base_info );
    
	// Domain extension wants to roam around the XTree, consulting
	// parents, children, anything really. So we need a separate pass.
	DBWalk::Actions actions2;
	actions2.push_back( domain_extension->GetInsertAction());
	db_walker.WalkZone( &actions2, zone, DBCommon::RootOrdinal::MAIN, DBWalk::WIND_IN, base_info );

	while(!de_extra_delete_queue.empty())
	{
		ExtraTreeDelete( de_extra_delete_queue.front() );
		de_extra_delete_queue.pop();
	}
	while(!de_extra_insert_queue.empty())
	{
		ExtraTreeInsert( de_extra_insert_queue.front() );
		de_extra_insert_queue.pop();
	}
}


void XTreeDatabase::ExtraTreeDelete(XLink xlink)
{		
    ASSERT( xlink != main_root_xlink );
    DBCommon::RootId root_id = roots_by_xlink[xlink];

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
    db_walker.WalkTree( &actions, xlink, root_id, DBWalk::WIND_OUT );   
    
   	roots_by_xlink.erase(xlink);
	roots_by_id.erase(root_id);
}


void XTreeDatabase::ExtraTreeInsert(DBCommon::RootOrdinal tree_id)
{		
    ASSERT( tree_id >= DBCommon::RootOrdinal::EXTRAS );
    XLink xlink = roots_by_id[tree_id];
    INDENT("e");   
    
	DBWalk::Actions actions;
    actions.push_back( domain->GetInsertAction() );
    actions.push_back( link_table->GetInsertAction() );
    actions.push_back( node_table->GetInsertAction() );
    actions.push_back( orderings->GetInsertAction() );
	db_walker.WalkTree( &actions, xlink, roots_by_xlink[xlink], DBWalk::WIND_IN );

	DBWalk::Actions actions2;
	actions2.push_back( domain_extension->GetInsertAction());
	db_walker.WalkTree( &actions2, xlink, roots_by_xlink[xlink], DBWalk::WIND_IN );
}


const DomainExtensionChannel *XTreeDatabase::GetDEChannel( const DomainExtension::Extender *extender ) const
{
    ASSERT( main_root_xlink );
	return domain_extension->GetChannel(extender);
}


void XTreeDatabase::PostUpdateActions()
{
    ASSERT( main_root_xlink );
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
    ASSERT( main_root_xlink )("XTreeDatabase@%p has no main tree", this);
	return link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
    ASSERT( main_root_xlink );
	return link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( main_root_xlink );
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
	return main_root_xlink.GetChildTreePtr();
}


XLink XTreeDatabase::GetMainRootXLink() const
{
	return main_root_xlink;
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
