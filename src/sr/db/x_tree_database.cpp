#include "x_tree_database.hpp"
#include "../sc_relation.hpp"
#include "../agents/agent.hpp"
#include "../vn_step.hpp"
#include "../sym/expression.hpp"
#include "../sym/predicate_operators.hpp"
#include "lacing.hpp"

using namespace SR;    

//#define TEST_RELATION_PROPERTIES_USING_DOMAIN
//#define TRACE_X_TREE_DB_DELTAS

#ifdef TRACE_X_TREE_DB_DELTAS
// Global because there are different x_tree_dbs owned by different SCR Engines
XTreeDatabase::CategoryOrderedIndex previous_category_ordered_domain;
#endif    


XTreeDatabase::XTreeDatabase( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    plan( clauses )
{ 
}


XTreeDatabase::Plan::Plan( const set< shared_ptr<SYM::BooleanExpression> > &clauses ) :
    indexes( make_shared<Indexes>(clauses) ),
    tables( make_shared<Tables>(indexes) )
{
}

    
void XTreeDatabase::Clear()
{
    // Clear everything 
    plan.indexes->unordered_domain.clear();
    plan.indexes->depth_first_ordered_index.clear();
    plan.indexes->category_ordered_index.clear();
    plan.indexes->simple_compare_ordered_index.clear();
    plan.indexes->domain_extension_classes = make_shared<SimpleCompareQuotientSet>();
    
    plan.tables->xlink_table.clear();
    plan.tables->node_table.clear();    
}


void XTreeDatabase::FullBuild( XLink root_xlink_ )
{      
	ASSERT( root_xlink_ );

	Clear();

	root_xlink = root_xlink_;		
	
    plan.tables->FullBuild(root_xlink);
            
#ifdef TRACE_X_TREE_DB_DELTAS
    if( Tracer::IsEnabled() ) 
    {        
        TRACE("X tree db rebuilt at ")(root_xlink)(":\n")
             ( category_ordered_index )("\n");
        previous_category_ordered_domain = category_ordered_index;
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


void XTreeDatabase::ExtendDomainNewPattern( PatternLink root_plink )
{
	plan.indexes->ExtendDomainNewPattern( this, root_plink );
}


void XTreeDatabase::ExtendDomainNewX()
{
	plan.indexes->ExtendDomainNewX( this );
}


const Tables::Row &XTreeDatabase::GetRow(XLink xlink) const
{
	return plan.tables->GetRow(xlink);
}


bool XTreeDatabase::HasRow(XLink xlink) const
{
	return plan.tables->HasRow(xlink);
}


const Tables::NodeRow &XTreeDatabase::GetNodeRow(TreePtr<Node> node) const
{
	return plan.tables->GetNodeRow(node);
}


bool XTreeDatabase::HasNodeRow(TreePtr<Node> node) const
{
	return plan.tables->HasNodeRow(node);
}


const Indexes &XTreeDatabase::GetIndexes() const
{
	return *plan.indexes;
}


Indexes &XTreeDatabase::GetIndexes()
{
	return *plan.indexes;
}


Tables &XTreeDatabase::GetTables()
{
	return *plan.tables;
}


set<TreeKit::LinkInfo> XTreeDatabase::GetDeclarers( TreePtr<Node> node ) const
{
    set<LinkInfo> infos;
   
    if( plan.tables->node_table.count(node)==0 ) // not found
        throw UnknownNode();
        
    Tables::NodeRow row = plan.tables->node_table.at(node);
    // Note that row.declarers is "precise", i.e. the XLinks are the actual
    // declaring xlinks, not just arbitrary parent links to the declaree.
    // Also correct for parallel links where only some declare.
    for( XLink declarer_xlink : row.declarers )
    {
        LinkInfo info;
        
        // first is TreePtr to the declarer node. Loses info about which 
        // link declared (in case of parallel links) but gets you the declarer node.
        info.first = plan.tables->xlink_table.at(declarer_xlink).parent_xlink.GetChildX();

        // second is TreePtrInterface * to the declarer's pointer to declaree
        // Retains precise info about which link.
        info.second = declarer_xlink.GetXPtr();
        
        infos.insert( info );
    }
    return infos;
}
