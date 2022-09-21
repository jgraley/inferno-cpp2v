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

    
void XTreeDatabase::FullClear()
{
    // Clear everything 
    plan.domain->ClearMonolithic();
    plan.indexes->ClearMonolithic();
    plan.link_table->ClearMonolithic();
    plan.node_table->ClearMonolithic();    
    
    Zone root_zone( root_xlink );
    plan.domain->Delete(root_zone);
    plan.indexes->Delete(root_zone);
    plan.link_table->Delete(root_zone);
    plan.node_table->Delete(root_zone);    
}


void XTreeDatabase::FullBuild( XLink root_xlink_ )
{      
	FullClear();

	root_xlink = root_xlink_;		    
	
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
	FullBuild(root_xlink);
}


void XTreeDatabase::ClearIncremental(XLink base_xlink)
{
}


void XTreeDatabase::BuildIncremental(XLink base_xlink)
{
}


XLink XTreeDatabase::UniquifyDomainExtension( XLink xlink )
{
	return plan.domain->UniquifyDomainExtension(xlink);
}


XLink XTreeDatabase::FindDomainExtension( XLink xlink ) const
{
	return plan.domain->FindDomainExtension(xlink);
}


void XTreeDatabase::ExtendDomainNewPattern( PatternLink root_plink )
{
	plan.domain->ExtendDomainNewPattern( *this, root_plink );
}


void XTreeDatabase::ExtendDomainNewX()
{
	plan.domain->ExtendDomainNewX( *this );
}


const LinkTable::Row &XTreeDatabase::GetRow(XLink xlink) const
{
	return plan.link_table->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
	return plan.link_table->HasRow(xlink);
}


const NodeTable::Row &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
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
