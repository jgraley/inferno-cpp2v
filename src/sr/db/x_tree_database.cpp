#include "x_tree_database.hpp"

using namespace SR;    

// We won't normally expect matches as postconditions to our
// public methods because changing strategies make some do more
// and others do less. But:
// - FullX methods should erase history, so can always check
// - All methods should match if strategy does not change, as
//   when testing the test
//#define DB_TEST_THE_TEST

XTreeDatabase::XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses )
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


XTreeDatabase::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    indexes( make_shared<Indexes>(clauses) ),
    domain( make_shared<Domain>() ),
    link_table( make_shared<LinkTable>() ),
    node_table( make_shared<NodeTable>() )
#ifdef DB_ENABLE_COMPARATIVE_TEST
    ,ref_domain( make_shared<Domain>() ),
    ref_indexes( make_shared<Indexes>(clauses, true) )
#endif    
{
}

    
void XTreeDatabase::SetRoot(XLink root_xlink_)
{
  	root_xlink = root_xlink_;		    
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
    plan.indexes->MonolithicClear();
    plan.link_table->MonolithicClear();
    plan.node_table->MonolithicClear();    
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        plan.ref_domain->MonolithicClear();
        plan.ref_indexes->MonolithicClear();
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
    plan.indexes->PrepareMonolithicBuild( actions );
    plan.link_table->PrepareMonolithicBuild( actions );
    plan.node_table->PrepareMonolithicBuild( actions );
    db_walker.FullWalk( &actions, root_xlink );
#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, false );
        plan.ref_indexes->PrepareMonolithicBuild( ref_actions );
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
	plan.indexes->PrepareMonolithicBuild( actions );
	plan.link_table->PrepareMonolithicBuild( actions );
	plan.node_table->PrepareMonolithicBuild( actions );
	db_walker.ExtraZoneWalk( &actions, extra_base_xlink );

#ifdef DB_ENABLE_COMPARATIVE_TEST
    {
        INDENT("⦼");
        DBWalk::Actions ref_actions;
        plan.ref_domain->PrepareMonolithicBuild( ref_actions, true );
        plan.ref_indexes->PrepareMonolithicBuild( ref_actions );
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


XLink XTreeDatabase::UniquifyDomainExtension( XLink xlink )
{
    ASSERT( root_xlink );
	return plan.domain->UniquifyDomainExtension(xlink);
}


XLink XTreeDatabase::FindDomainExtension( TreePtr<Node> node ) const
{
    ASSERT( root_xlink );
	return plan.domain->FindDomainExtension(node);
}


void XTreeDatabase::ExtendDomainNewPattern( PatternLink root_plink )
{
    ASSERT( root_xlink );
	plan.domain->ExtendDomainNewPattern( *this, root_plink );
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


const LinkTable::Row &XTreeDatabase::GetRow(XLink xlink) const
{
    ASSERT( root_xlink );
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


const Indexes &XTreeDatabase::GetIndexes() const
{
	return *plan.indexes;
}


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
        info.first = plan.link_table->GetRow(declarer_xlink).parent_xlink.GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
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

