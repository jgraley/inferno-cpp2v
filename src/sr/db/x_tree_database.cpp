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
    node_table( make_shared<NodeTable>() ),
    link_table( make_shared<LinkTable>() ),
    orderings( make_shared<Orderings>(lacing, this) ),
    domain_extension( make_shared<DomainExtension>(this, domain_extenders) ),    
    main_root_xlink( main_root_xlink_ )
{
    auto on_insert_extra_tree = [=](XLink extra_root)
    {
        de_extra_queue.push(extra_root);
    };

    auto on_delete_extra_tree = [=](XLink extra_root)
	{
        de_extra_queue.push(extra_root);
    };
    
    domain_extension->SetOnExtraTreeFunctions( on_insert_extra_tree, 
                                                     on_delete_extra_tree );
}

    
void XTreeDatabase::InitialBuild()
{      
    INDENT("p");
    ASSERT( main_root_xlink );
    ASSERT( de_extra_queue.empty() );
	
    DBWalk::Actions actions;
    domain->PrepareInsert( actions );
    orderings->PrepareInsert( actions );
    link_table->PrepareInsert( actions );
    node_table->PrepareInsert( actions );
    
	roots[main_root_xlink] = { DBCommon::RootOrdinal::MAIN };
	roots[XLink::MMAX_Link] = { DBCommon::RootOrdinal::MMAX };
	roots[XLink::OffEndXLink] = { DBCommon::RootOrdinal::OFF_END };
	next_root_ordinal = DBCommon::RootOrdinal::EXTRAS;
	
	for( auto p : roots )
		db_walker.Walk( &actions, p.first, DBWalk::ROOT, &p.second );

    domain_extension->InitialBuild();
    
    while(!de_extra_queue.empty())
    {
		InsertExtraTree( de_extra_queue.front() );
		de_extra_queue.pop();
	}    
}


void XTreeDatabase::UpdateMainTree( const TreeZone &target_tree_zone, const FreeZone &source_free_zone )
{
	ASSERT( target_tree_zone.GetNumTerminii() == source_free_zone.GetNumTerminii() );	
	ASSERT( target_tree_zone.GetNumTerminii() == 0 ); // TODO under #723

    // Update database 
    DeleteMainTree( target_tree_zone.GetBaseXLink(), true );    

    // Patch the tree
    // acts on all copies of the xlink, due to indirection, possibly including root as held by db 
    target_tree_zone.GetBaseXLink().SetXPtr( source_free_zone.GetBaseNode() ); 

    // Update database 
    InsertMainTree( target_tree_zone.GetBaseXLink(), true );   	
}


void XTreeDatabase::PreUpdateMainTree( const TreeZone &target_tree_zone )
{
	ASSERT( target_tree_zone.GetNumTerminii() == 0 ); // Should be everything

	target_tree_zone.DBCheck(this); // Move back to UpdateMainTree once this is empty
        
    // Update database 
    DeleteMainTree( target_tree_zone.GetBaseXLink(), false );    
}


void XTreeDatabase::PostUpdateMainTree( const TreeZone &target_tree_zone )
{   
	ASSERT( target_tree_zone.GetNumTerminii() == 0 ); // Should be everything

    // Update database 
    InsertMainTree( target_tree_zone.GetBaseXLink(), false );   	
}


void XTreeDatabase::InsertMainTree(XLink xlink, bool incremental)
{
    INDENT("i");
    
    ASSERT( de_extra_queue.empty() );
    
	DBWalk::Context context;
	if( xlink == main_root_xlink ) 	
		context = DBWalk::Context::ROOT;
	else
		context = DBWalk::Context::BASE;	

    DBWalk::Actions actions;
    if( !incremental )
    {
		domain->PrepareInsert( actions );
		orderings->PrepareInsert( actions );
		link_table->PrepareInsert( actions );
		node_table->PrepareInsert( actions );
	}
    db_walker.Walk( &actions, xlink, context, &roots[main_root_xlink] );
    
    if( !incremental )
    {
		// Domain extension wants to roam around the XTree, consulting
		// parents, children, anything really. So we need a separate pass.
		DBWalk::Actions actions2;
		domain_extension->PrepareInsert( actions2 );
		db_walker.Walk( &actions2, xlink, context, &roots[main_root_xlink] );
	
		while(!de_extra_queue.empty())
		{
			InsertExtraTree( de_extra_queue.front() );
			de_extra_queue.pop();
		}
	}
}


void XTreeDatabase::DeleteMainTree(XLink xlink, bool incremental)
{
    INDENT("d");

    ASSERT( de_extra_queue.empty() );

	DBWalk::Context context;
	if( xlink == main_root_xlink ) 	
		context = DBWalk::Context::ROOT;
	else
		context = DBWalk::Context::BASE;	
	
    DBWalk::Actions actions;
    if( !incremental )
    {
		domain->PrepareDelete( actions );
		domain_extension->PrepareDelete( actions );
		orderings->PrepareDelete( actions );
		link_table->PrepareDelete( actions );
		node_table->PrepareDelete( actions );
	}
    // TODO be able to supply ROOT or the new BASE depending on whether 
    // we're being asked to act at a root. Fix up eg in link table where 
    // we need to tolerate multiple calls at ROOT not just one at InitalBuild()
    db_walker.Walk( &actions, xlink, context, &roots[main_root_xlink] );   

    if( !incremental )
    {
		while(!de_extra_queue.empty())
		{
			DeleteExtraTree( de_extra_queue.front() );
			de_extra_queue.pop();
		}
	}
}


void XTreeDatabase::InsertExtraTree(XLink xlink)
{
    INDENT("e");
    
    ASSERT( xlink != main_root_xlink );
    ASSERT( roots.count(xlink) == 0 );
    
	roots[xlink] = { next_root_ordinal };    
    ((int &)next_root_ordinal)++;
    
	DBWalk::Actions actions;
	domain->PrepareInsert( actions );
	orderings->PrepareInsert( actions );
	link_table->PrepareInsert( actions );
	node_table->PrepareInsert( actions );
	db_walker.Walk( &actions, xlink, DBWalk::Context::ROOT, &roots[xlink] );

	DBWalk::Actions actions2;
	domain_extension->PrepareInsert( actions2 );
	db_walker.Walk( &actions2, xlink, DBWalk::Context::ROOT, &roots[xlink] );
}


void XTreeDatabase::DeleteExtraTree(XLink xlink)
{		
    ASSERT( xlink != main_root_xlink );
    ASSERT( roots.count(xlink) == 1 );

    // Note not symmetrical with InsertExtra(): we
    // will be invoked with every xlink in the extra
    // zones and on each call we delete just that
    // xlink.
    DBWalk::Actions actions;
    domain->PrepareDelete( actions );
    orderings->PrepareDelete( actions );
    link_table->PrepareDelete( actions );
    node_table->PrepareDelete( actions );
    domain_extension->PrepareDelete( actions );
    db_walker.Walk( &actions, xlink, DBWalk::Context::ROOT, &roots[xlink] );   
    
   	roots.erase(xlink);
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
