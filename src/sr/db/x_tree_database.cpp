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
	auto on_insert_extra_subtree = [&](XLink extra_base_xlink)
	{
        TRACEC("Inserting extra subtree to x tree db, base: ")(extra_base_xlink)("\n");
        MonolithicExtra( extra_base_xlink );
        InsertExtra( extra_base_xlink );        
    };

	auto on_delete_extra_xlink = [&](XLink extra_xlink)
	{
        TRACEC("Deleting extra xlink from x tree db: ")(extra_xlink)("\n");
        DeleteExtra( extra_xlink );
        // No monolithic variant: everything monolithic gets splatted in ClearMonolithic()
    };
    
    plan.domain->SetOnExtraXLinkFunctions( on_insert_extra_subtree,
                                           on_delete_extra_xlink );
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
    InsertPermanent();
    TreeZone root_zone( root_xlink );
    Insert( root_zone );
    
#ifdef DB_ENABLE_COMPARATIVE_TEST
    ExpectMatches();
#endif
}


void XTreeDatabase::MonolithicClear()
{
    INDENT("c");
    plan.domain->MonolithicClear();
    plan.link_table->MonolithicClear();
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
    plan.link_table->PrepareMonolithicBuild( actions );
    plan.node_table->PrepareMonolithicBuild( actions );
    db_walker.FullWalk( &actions, root_xlink );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, false );
        db_walker.FullWalk( &ref_actions, root_xlink );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }   
#endif
}


void XTreeDatabase::InsertPermanent()
{
    INDENT("p");
    
    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.indexes->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    db_walker.InitWalk( &actions );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        db_walker.InitWalk( &ref_actions );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::MonolithicExtra(XLink extra_base_xlink)
{
    INDENT("f");

    DBWalk::Actions actions;
	plan.domain->PrepareMonolithicBuild( actions, true );
	plan.link_table->PrepareMonolithicBuild( actions );
	plan.node_table->PrepareMonolithicBuild( actions );
	db_walker.ExtraZoneWalk( &actions, extra_base_xlink );

#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, true );
        db_walker.ExtraZoneWalk( &ref_actions, extra_base_xlink );
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
    db_walker.ZoneWalk( &actions, zone );   
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareDelete( ref_actions );
        plan.ref_indexes->PrepareDelete( ref_actions );
        db_walker.ZoneWalk( &ref_actions, zone );
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
    db_walker.ZoneWalk( &actions, zone );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        db_walker.ZoneWalk( &ref_actions, zone );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::InsertExtra(XLink extra_base_xlink)
{
    INDENT("e");
    
	DBWalk::Actions actions;
	plan.domain->PrepareInsert( actions );
	plan.indexes->PrepareInsert( actions );
	plan.link_table->PrepareInsert( actions );
	plan.node_table->PrepareInsert( actions );
	db_walker.ExtraZoneWalk( &actions, extra_base_xlink );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareInsert( ref_actions );
        plan.ref_indexes->PrepareInsert( ref_actions );
        db_walker.ExtraZoneWalk( &ref_actions, extra_base_xlink );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
}


void XTreeDatabase::DeleteExtra(XLink extra_xlink)
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
    db_walker.SingleXLinkWalk( &actions, extra_xlink );   
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareDelete( ref_actions );
        plan.ref_indexes->PrepareDelete( ref_actions );
        db_walker.SingleXLinkWalk( &ref_actions, extra_xlink );
#ifdef DB_TEST_THE_TEST
        ExpectMatches();
#endif
    }
#endif
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
//	return plan.link_table->GetRow(xlink).parent_xlink;
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
