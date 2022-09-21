#include "x_tree_database.hpp"

using namespace SR;    


XTreeDatabase::XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses )
{
	auto on_extra_xlink = [&](XLink extra_xlink)
	{
        TRACEC(extra_xlink)("\n");
        
		DBWalk::Actions actions;
      	plan.domain->PrepareBuildMonolithic( actions );
        plan.indexes->PrepareBuildMonolithic( actions );
        plan.link_table->PrepareBuildMonolithic( actions );
        plan.node_table->PrepareBuildMonolithic( actions );
		plan.domain->PrepareInsert( actions );
		plan.indexes->PrepareInsert( actions );
		plan.link_table->PrepareInsert( actions );
		plan.node_table->PrepareInsert( actions );
		db_walker.ExtraXLinkWalk( actions, extra_xlink );
    };
    
    plan.domain->SetOnExtraXLinkFunction(on_extra_xlink);
}


XTreeDatabase::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    indexes( make_shared<Indexes>(clauses) ),
    domain( make_shared<Domain>() ),
    link_table( make_shared<LinkTable>() ),
    node_table( make_shared<NodeTable>() )
{
}

    
void XTreeDatabase::SetRoot(XLink root_xlink_)
{
  	root_xlink = root_xlink_;		    
}
    
    
void XTreeDatabase::FullClear()
{
    ASSERT( root_xlink );
    
    // Clear everything 
    ClearMonolithic();
    
    Zone root_zone( root_xlink );
    Delete( root_zone );
}


void XTreeDatabase::FullBuild()
{      
    ASSERT( root_xlink );

	FullClear();
	
	{
        DBWalk::Actions actions;
        plan.domain->PrepareBuildMonolithic( actions );
        plan.indexes->PrepareBuildMonolithic( actions );
        plan.link_table->PrepareBuildMonolithic( actions );
        plan.node_table->PrepareBuildMonolithic( actions );
        db_walker.FullWalk( actions, root_xlink );
    }
    
	{
        Zone root_zone( root_xlink );
        DBWalk::Actions actions;
        plan.domain->PrepareInsert( actions );
        plan.indexes->PrepareInsert( actions );
        plan.link_table->PrepareInsert( actions );
        plan.node_table->PrepareInsert( actions );
        db_walker.ZoneWalk( actions, root_zone );
    }
            
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) 
    {        
        TRACE("X tree db rebuilt at ")(root_xlink)(":\n")
             ( plan.domain->unordered_domain )("\n");
        plan.domain->previous_unordered_domain = plan.domain->unordered_domain;
    }
#endif    
}


void XTreeDatabase::BuildMonolithic()
{
    ASSERT( root_xlink );
    
    ClearMonolithic();
    
    DBWalk::Actions actions;
    plan.domain->PrepareBuildMonolithic( actions );
    plan.indexes->PrepareBuildMonolithic( actions );
    plan.link_table->PrepareBuildMonolithic( actions );
    plan.node_table->PrepareBuildMonolithic( actions );
    db_walker.FullWalk( actions, root_xlink );
}


void XTreeDatabase::ClearMonolithic()
{
    plan.domain->ClearMonolithic();
    plan.indexes->ClearMonolithic();
    plan.link_table->ClearMonolithic();
    plan.node_table->ClearMonolithic();    
}


void XTreeDatabase::Delete(const Zone &zone)
{
    plan.domain->Delete(zone);
    plan.indexes->Delete(zone);
    plan.link_table->Delete(zone);
    plan.node_table->Delete(zone);    
}


void XTreeDatabase::Insert(const Zone &zone)
{
    DBWalk::Actions actions;
    plan.domain->PrepareInsert( actions );
    plan.indexes->PrepareInsert( actions );
    plan.link_table->PrepareInsert( actions );
    plan.node_table->PrepareInsert( actions );
    db_walker.ZoneWalk( actions, zone );
}


XLink XTreeDatabase::UniquifyDomainExtension( XLink xlink )
{
    ASSERT( root_xlink );
	return plan.domain->UniquifyDomainExtension(xlink);
}


XLink XTreeDatabase::FindDomainExtension( XLink xlink ) const
{
    ASSERT( root_xlink );
	return plan.domain->FindDomainExtension(xlink);
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


Indexes &XTreeDatabase::GetIndexes()
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
