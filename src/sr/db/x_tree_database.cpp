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

XTreeDatabase::XTreeDatabase( shared_ptr<Lacing> lacing, XLink root_xlink_ ) :
    plan( this, lacing ),
    root_xlink( root_xlink_ )
{
    auto on_insert_extra_zone = [=](const TreeZone &extra_zone)
    {
        MonolithicExtraZone( extra_zone );
        InsertExtraZone( extra_zone );        
    };

    auto on_delete_extra_zone = [=](const TreeZone &extra_zone)
	{
        DeleteExtraZone( extra_zone );
        // No monolithic variant: everything monolithic gets splatted in ClearMonolithic()
    };
    
    plan.domain->SetOnExtraXLinkFunctions( on_insert_extra_zone, 
                                           on_delete_extra_zone );
}


XTreeDatabase::Plan::Plan( const XTreeDatabase *algo, shared_ptr<Lacing> lacing ) :
    domain( make_shared<Domain>() ),
    node_table( make_shared<NodeTable>() ),
    link_table( make_shared<LinkTable>() ),
    indexes( make_shared<Indexes>(lacing, algo) )
#ifdef DB_ENABLE_COMPARATIVE_TEST
    ,ref_domain( make_shared<Domain>() ),
    ref_indexes( make_shared<Indexes>(lacing, algo, true) )
#endif    
{
}

    
void XTreeDatabase::InitialBuild()
{      
    ASSERT( root_xlink );
	
    // Full build monolithically
    MonolithicBuild();

    // Full build incrementally
    InitialBuildForIncremental();
    
#ifdef DB_ENABLE_COMPARATIVE_TEST
    ExpectMatches();
#endif
}


void XTreeDatabase::MonolithicClear()
{
    INDENT("c");
    plan.domain->MonolithicClear();
    plan.node_table->MonolithicClear();    
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        plan.ref_domain->MonolithicClear();
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::MonolithicBuild()
{
    INDENT("m");
    ASSERT( root_xlink );

    DBWalk::Actions actions;
    plan.domain->PrepareMonolithicBuild( actions, false );
    plan.node_table->PrepareMonolithicBuild( actions );
    InitialWalk( &actions, root_xlink );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, false );
        InitialWalk( &ref_actions, root_xlink );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }   
#endif
}


void XTreeDatabase::InitialBuildForIncremental()
{
    INDENT("p");
    
    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.indexes->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    InitialWalk( &actions, root_xlink );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        InitialWalk( &ref_actions, root_xlink );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::MonolithicExtraZone(const TreeZone &extra_zone)
{
    INDENT("f");

    DBWalk::Actions actions;
	plan.domain->PrepareMonolithicBuild( actions, true );
	plan.node_table->PrepareMonolithicBuild( actions );
	db_walker.Walk( &actions, extra_zone, DBWalk::ROOT );

#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, true );
        db_walker.Walk( &ref_actions, extra_zone, DBWalk::ROOT );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::Delete(const TreeZone &zone)
{
    INDENT("d");

    DBWalk::Actions actions;
    plan.domain->PrepareDelete( actions );
    plan.indexes->PrepareDelete( actions );
    plan.link_table->PrepareDelete( actions );
    plan.node_table->PrepareDelete( actions );
    db_walker.Walk( &actions, zone, DBWalk::UNKNOWN );   
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareDelete( ref_actions );
        plan.ref_indexes->PrepareDelete( ref_actions );
        db_walker.Walk( &ref_actions, zone, DBWalk::UNKNOWN );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::Insert(const TreeZone &zone)
{
    INDENT("i");

    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.indexes->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    db_walker.Walk( &actions, zone, DBWalk::UNKNOWN );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        db_walker.Walk( &ref_actions, zone, DBWalk::UNKNOWN );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::InsertExtraZone(const TreeZone &extra_zone)
{
    INDENT("e");
    
	DBWalk::Actions actions;
	plan.domain->PrepareInsert( actions );
	plan.indexes->PrepareInsert( actions );
	plan.link_table->PrepareInsert( actions );
	plan.node_table->PrepareInsert( actions );
	db_walker.Walk( &actions, extra_zone, DBWalk::ROOT );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        db_walker.Walk( &ref_actions, extra_zone, DBWalk::ROOT );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::DeleteExtraZone(const TreeZone &extra_zone)

{
    // Note not symmetrical with InsertExtra(): we
    // will be invoked with every xlink in the extra
    // zones and on each call we delete just that
    // xlink.
    DBWalk::Actions actions;
    plan.domain->PrepareDelete( actions );
    plan.indexes->PrepareDelete( actions );
    plan.link_table->PrepareDelete( actions );
    plan.node_table->PrepareDelete( actions );
    db_walker.Walk( &actions, extra_zone, DBWalk::ROOT );   
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareDelete( ref_actions );
        plan.ref_indexes->PrepareDelete( ref_actions );
        db_walker.Walk( &ref_actions, extra_zone, DBWalk::ROOT );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}

void XTreeDatabase::InitialWalk( const DBWalk::Actions *actions,
                                 XLink root_xlink )
{
    db_walker.Walk( actions, TreeZone( XLink::MMAX_Link ), DBWalk::ROOT );
    db_walker.Walk( actions, TreeZone( XLink::OffEndXLink ), DBWalk::ROOT );
    db_walker.Walk( actions, TreeZone( root_xlink ), DBWalk::ROOT );
}                                 


XLink XTreeDatabase::UniquifyDomainExtension( TreePtr<Node> node, bool expect_in_domain )
{
    ASSERT( root_xlink );
	return plan.domain->UniquifyDomainExtension(node, expect_in_domain);
}


void XTreeDatabase::ExtendDomainNewPattern( PatternLink root_plink )
{
    ASSERT( root_xlink );
	plan.domain->ExtendDomainNewPattern( *this, root_plink );
	
    if( ReadArgs::test_rel )
    {
		plan.domain->TestRelations( plan.domain->unordered_domain );
		plan.indexes->TestRelations( plan.domain->unordered_domain );
	}		
}


void XTreeDatabase::ExtendDomainNewX()
{
    ASSERT( root_xlink );
	plan.domain->ExtendDomainNewX( *this );
}


void XTreeDatabase::UnExtendDomain()
{
    ASSERT( root_xlink );
    plan.domain->UnExtendDomain();
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
    ASSERT( root_xlink )("XTreeDatabase@%p has no root xlnik", this);
	return plan.link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
    ASSERT( root_xlink );
	return plan.link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( root_xlink );
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


const Indexes &XTreeDatabase::GetIndexes() const
{
	return *plan.indexes;
}


TreePtr<Node> XTreeDatabase::GetRootNode() const
{
	return root_xlink.GetChildX();
}


XLink XTreeDatabase::GetRootXLink() const
{
	return root_xlink;
}


void XTreeDatabase::Dump() const
{
    plan.indexes->Dump();
}


#ifdef DB_ENABLE_COMPARATIVE_TEST
void XTreeDatabase::ExpectMatches() const
{
    plan.ref_indexes->ExpectMatching( *plan.indexes );
}
#endif



set<TreeKit::LinkInfo> XTreeDatabase::GetDeclarers( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( !plan.node_table->HasRow(node) ) // not found
        throw UnknownNode();
        
    NodeTable::Row row = plan.node_table->GetRow(node);
    // Note that row.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : row.declarers )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = TryGetParentXLink(declarer_xlink).GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}
