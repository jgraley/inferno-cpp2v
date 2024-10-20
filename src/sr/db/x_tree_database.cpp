#include "x_tree_database.hpp"

#include "lacing.hpp"

#include "common/read_args.hpp"

using namespace SR;    

// We won't normally expect matches as postconditions to our
// public methods because changing strategies make some do more
// and others do less. But:
// - FullX methods should erase history, so can always check
// - All methods should match if strategy does not change, as
//   when testing the test
//#define DB_TEST_THE_TEST

XTreeDatabase::XTreeDatabase( XLink main_root_xlink_, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders ) :
    plan( this, lacing, domain_extenders ),
    main_root_xlink( main_root_xlink_ )
{
    auto on_insert_extra_tree = [=](XLink extra_base)
    {
        InsertExtraTree( extra_base );        
    };

    auto on_delete_extra_tree = [=](XLink extra_base)
	{
        DeleteExtraTree( extra_base );
    };
    
    plan.domain_extension->SetOnExtraTreeFunctions( on_insert_extra_tree, 
                                                     on_delete_extra_tree );
}


XTreeDatabase::Plan::Plan( const XTreeDatabase *algo, shared_ptr<Lacing> lacing, DomainExtension::ExtenderSet domain_extenders ) :
    domain( make_shared<Domain>() ),
    node_table( make_shared<NodeTable>() ),
    link_table( make_shared<LinkTable>() ),
    orderings( make_shared<Orderings>(lacing, algo) ),
    domain_extension( make_shared<DomainExtension>(algo, domain_extenders) )
{
}

    
void XTreeDatabase::InitialBuild()
{      
    INDENT("p");
    ASSERT( main_root_xlink );
	
	// Full build incrementally
    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.orderings->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    InitialWalk( &actions, main_root_xlink );

    plan.domain_extension->InitialBuild();
}


void XTreeDatabase::Delete(XLink base_xlink)
{
    INDENT("d");

    DBWalk::Actions actions;
    plan.domain->PrepareDelete( actions );
    plan.domain_extension->PrepareDelete( actions );
    plan.orderings->PrepareDelete( actions );
    plan.link_table->PrepareDelete( actions );
    plan.node_table->PrepareDelete( actions );
    // TODO be able to supply ROOT or the new BASE depending on whether 
    // we're being asked to act at a root. Fix up eg in link table where 
    // we need to tolerate multiple calls at ROOT not just one at InitalBuild()
    db_walker.Walk( &actions, base_xlink, DBWalk::BASE );   
}


void XTreeDatabase::Insert(XLink base_xlink)
{
    INDENT("i");

    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.orderings->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    plan.domain_extension->PrepareInsert( actions );
    db_walker.Walk( &actions, base_xlink, DBWalk::BASE );
}


void XTreeDatabase::InsertExtraTree(XLink root_xlink)
{
    INDENT("e");
    
	DBWalk::Actions actions;
	plan.domain->PrepareInsert( actions );
	plan.orderings->PrepareInsert( actions );
	plan.link_table->PrepareInsert( actions );
	plan.node_table->PrepareInsert( actions );
	plan.domain_extension->PrepareInsertExtra( actions );
	db_walker.Walk( &actions, root_xlink, DBWalk::ROOT );
}


void XTreeDatabase::DeleteExtraTree(XLink root_xlink)
{
    // Note not symmetrical with InsertExtra(): we
    // will be invoked with every xlink in the extra
    // zones and on each call we delete just that
    // xlink.
    DBWalk::Actions actions;
    plan.domain->PrepareDelete( actions );
    plan.orderings->PrepareDelete( actions );
    plan.link_table->PrepareDelete( actions );
    plan.node_table->PrepareDelete( actions );
    plan.domain_extension->PrepareDeleteExtra( actions );
    db_walker.Walk( &actions, root_xlink, DBWalk::ROOT );   
}

void XTreeDatabase::InitialWalk( const DBWalk::Actions *actions,
                                 XLink main_root_xlink )
{
    db_walker.Walk( actions, main_root_xlink, DBWalk::ROOT );
    db_walker.Walk( actions, XLink::MMAX_Link, DBWalk::ROOT );
    db_walker.Walk( actions, XLink::OffEndXLink, DBWalk::ROOT );
}                                 


const DomainExtensionChannel *XTreeDatabase::GetDEChannel( const DomainExtension::Extender *extender ) const
{
    ASSERT( main_root_xlink );
	return plan.domain_extension->GetChannel(extender);
}


void XTreeDatabase::PostUpdateActions()
{
    ASSERT( main_root_xlink );
	plan.domain_extension->PostUpdateActions();
}

	
const Domain &XTreeDatabase::GetDomain() const
{
	return *plan.domain;
}


const LinkTable &XTreeDatabase::GetLinkTable() const
{
	return *plan.link_table;
}


const LinkTable::Row &XTreeDatabase::GetRow(XLink xlink) const
{
    ASSERT( main_root_xlink )("XTreeDatabase@%p has no main tree", this);
	return plan.link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
    ASSERT( main_root_xlink );
	return plan.link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( main_root_xlink );
	return plan.node_table->GetRow(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
	return plan.node_table->HasRow(node);
}


XLink XTreeDatabase::TryGetParentXLink(XLink xlink) const
{
	TreePtr<Node> parent_node = plan.link_table->GetRow(xlink).parent_node;
	if( !parent_node )
		return XLink();
		
	const set<XLink> &ps = plan.node_table->GetRow(parent_node).parents;

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
	TreePtr<Node> child_node = (TreePtr<Node>)*px;
	
    NodeTable::Row row = plan.node_table->GetRow(child_node);
    ASSERT( !row.parents.empty() )("child_node=")(child_node);

    for( XLink xlink : row.parents )
		if( xlink.GetXPtr() == px )
			return xlink;
	
    ASSERTFAIL(); // Could not find matching XLink even though there were candidates	
}


XLink XTreeDatabase::GetLastDescendant(XLink xlink) const
{
    TreePtr<Node> x = xlink.GetChildX();
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
	return *plan.orderings;
}


TreePtr<Node> XTreeDatabase::GetMainRootNode() const
{
	return main_root_xlink.GetChildX();
}


XLink XTreeDatabase::GetMainRootXLink() const
{
	return main_root_xlink;
}


bool XTreeDatabase::IsDirtyGrass( TreePtr<Node> node ) const
{
	return dirty_grass.count(node) > 0;
}


void XTreeDatabase::AddDirtyGrass( TreePtr<Node> node ) const
{
	dirty_grass.insert(node);
}


void XTreeDatabase::ClearDirtyGrass()
{
    dirty_grass.clear();
}


void XTreeDatabase::Dump() const
{
    plan.orderings->Dump();
}


bool XTreeDatabase::IsRequireReports() const
{
    return true; // Yes we do, we're the actual database, and must maintain domain extension info
}    


set<NavigationInterface::LinkInfo> XTreeDatabase::GetParents( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( !plan.node_table->HasRow(node) ) // not found
        throw UnknownNode();
        
    NodeTable::Row row = plan.node_table->GetRow(node);
    ASSERT( !row.parents.empty() )("node=")(node);
    for( XLink parent_xlink : row.parents )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = TryGetParentXLink(parent_xlink).GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = parent_xlink.GetXPtr();
        
        infos.insert( info );
    }
    ASSERT( !infos.empty() )("node=")(node);
    return infos;
}


set<NavigationInterface::LinkInfo> XTreeDatabase::GetDeclarers( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( !plan.node_table->HasRow(node) ) // not found
        throw UnknownNode();
        
    NodeTable::Row row = plan.node_table->GetRow(node);
    // Note that row.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : row.declarers ) // TODO give XLink a method to generate this
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Imprecise: loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = TryGetParentXLink(declarer_xlink).GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}


void XTreeDatabase::TestRelations()
{
    if( ReadArgs::test_rel )
    {
		plan.domain_extension->TestRelations( plan.domain->unordered_domain );
		plan.orderings->TestRelations( plan.domain->unordered_domain );
	}		
}
