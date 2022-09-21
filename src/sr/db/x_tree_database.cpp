#include "x_tree_database.hpp"

using namespace SR;    


XTreeDatabase::XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses )
{
	auto on_extra_xlink = [&](XLink extra_xlink)
	{
        TRACEC(extra_xlink)("\n");
        
		DBWalk::Actions actions;
		plan.domain->PrepareExtraXLink( actions );
		plan.indexes->PrepareExtraXLink( actions );
		plan.link_table->PrepareExtraXLink( actions );
		plan.node_table->PrepareExtraXLink( actions );
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

    
void XTreeDatabase::Clear()
{
    // Clear everything 
    plan.indexes->depth_first_ordered_index.clear();
    plan.indexes->category_ordered_index.clear();
    plan.indexes->simple_compare_ordered_index.clear();

    plan.domain->unordered_domain.clear();
    plan.domain->domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
    
    plan.link_table->xlink_table.clear();
    plan.node_table->node_table.clear();    
}


void XTreeDatabase::FullBuild( XLink root_xlink_ )
{      
	ASSERT( root_xlink_ );

	Clear();

	root_xlink = root_xlink_;		
	
	DBWalk::Actions actions;
	plan.domain->PrepareFullBuild( actions );
	plan.indexes->PrepareFullBuild( actions );
	plan.link_table->PrepareFullBuild( actions );
	plan.node_table->PrepareFullBuild( actions );
    db_walker.FullWalk( actions, root_xlink );
            
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) 
    {        
        TRACE("X tree db rebuilt at ")(root_xlink)(":\n")
             ( plan.domain->unordered_domain )("\n");
        plan.domain->previous_unordered_domain = plan.domain->unordered_domain;
    }
#endif    
}


void XTreeDatabase::UpdateRootXLink(XLink root_xlink_)
{
	root_xlink = root_xlink_;
}


void XTreeDatabase::BuildNonIncremental()
{
	FullBuild(root_xlink);
	ExtendDomainNewX();
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


const NodeTable::NodeRow &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
	return plan.node_table->GetNodeRow(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
	return plan.node_table->HasNodeRow(node);
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
   
    if( plan.node_table->node_table.count(node)==0 ) // not found
        throw UnknownNode();
        
    NodeTable::NodeRow row = plan.node_table->node_table.at(node);
    // Note that row.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : row.declarers )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = plan.link_table->xlink_table.at(declarer_xlink).parent_xlink.GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}
