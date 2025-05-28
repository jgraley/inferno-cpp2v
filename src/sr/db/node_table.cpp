#include "node_table.hpp"
#include "sc_relation.hpp"
#include "link_table.hpp"

#include "common/read_args.hpp"

using namespace SR;    


NodeTable::NodeTable(const LinkTable *link_table_) :
	link_table( link_table_ )
{
}


const NodeTable::Row &NodeTable::GetRow(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasRow(node) )("\n")
          ("X tree database: no node row for ")(node)("\n")
          ("Node table: ")(rows);
    return rows.at(node);
}


bool NodeTable::HasRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return rows.count(node) > 0;
}


bool NodeTable::IsDeclarer(XLink xlink) const
{
	LinkTable::Row link_row = link_table->GetRow(xlink);
    switch( link_row.context_type )
    {
        case DBCommon::SINGULAR:
        case DBCommon::IN_SEQUENCE:
        case DBCommon::IN_COLLECTION:
        {
            set<const TreePtrInterface *> declared = link_row.parent_node->GetDeclared();
            return declared.count( xlink.GetTreePtrInterface() ) > 0;
        }
        default:
        {
            return false;
        }
    }
}


void NodeTable::InsertTree(TreeZone &zone)
{     
	// For building, we must add every node in the zone
	auto action = [&](const DBWalk::WalkInfo &walk_info)
	{
		 InsertLink(walk_info.xlink);
	};
	db_walker.WalkTreeZone( action, zone, DBWalk::WIND_IN );
}


void NodeTable::DeleteTree(TreeZone &zone)
{
	// For tear-down, we must remove every node in the zone
	auto action = [&](const DBWalk::WalkInfo &walk_info)
	{
		 DeleteLink(walk_info.xlink);
	};
	db_walker.WalkTreeZone( action, zone, DBWalk::WIND_OUT);
}


NodeTable::RAIISuspendForSwap::RAIISuspendForSwap(NodeTable *node_table_, TreeZone &zone1_, TreeZone &zone2_ ) :
	RAIISuspendForSwapBase( zone1_, zone2_ ),
	node_table( *node_table_ )
{	
	// For swaps, we only need to act at the boundary of the zone
	node_table.DeleteLink(zone1.GetBaseXLink());
	for( XLink terminus : zone1.GetTerminusXLinks() )
		node_table.DeleteLink(terminus);	
	node_table.DeleteLink(zone2.GetBaseXLink());
	for( XLink terminus : zone2.GetTerminusXLinks() )
		node_table.DeleteLink(terminus);	
}


NodeTable::RAIISuspendForSwap::~RAIISuspendForSwap()
{
	// For swaps, we only need to act at the boundary of the zone
	node_table.InsertLink(zone1.GetBaseXLink());
	for( XLink terminus : zone1.GetTerminusXLinks() )
		node_table.InsertLink(terminus);
	node_table.InsertLink(zone2.GetBaseXLink());
	for( XLink terminus : zone2.GetTerminusXLinks() )
		node_table.InsertLink(terminus);
}



void NodeTable::InsertLink(XLink xlink)
{
	TreePtr<Node> node = xlink.GetChildTreePtr();

	// Create if not already there
	TRACE("NODE TABLE weakly inserts row for: ")(node)("\n");
	Row &row = rows[node];
	
	InsertSolo( row.incoming_xlinks, xlink );            
	if( IsDeclarer(xlink) )
	{
		InsertSolo( row.declaring_xlinks, xlink );
		//ASSERT( row.declaring_xlinks.size()==1 ); // can be removed, only here to help smoke out bugs
	}
}


void NodeTable::DeleteLink(XLink xlink)
{
	TreePtr<Node> node = xlink.GetChildTreePtr();

	// Should already be there
	Row &row = rows.at(node);
	
	EraseSolo( row.incoming_xlinks, xlink );            
	if( IsDeclarer(xlink) )
		EraseSolo( row.declaring_xlinks, xlink );
		
	if( !row.incoming_xlinks.empty() )
	{
		TRACE("NODE TABLE does not delete row due %u>0 remaining incoming: ", row.incoming_xlinks.size())(node)("\n");
		return;
	}
	
	TRACE("NODE TABLE deletes row for: ")(xlink)("\n");
	EraseSolo( rows, node );
}


vector<TreePtr<Node>> NodeTable::GetNodeDomainAsVector() const
{
    vector<TreePtr<Node>> v;
    for( auto p : rows )
        v.push_back(p.first);
    return v;
}


string NodeTable::Row::GetTrace() const
{
    string s = "(";
    
    s += " incoming=" + Trace(incoming_xlinks);
    s += " declaring=" + Trace(declaring_xlinks);
    
    s += ")";
    return s;
}


string NodeTable::GetTrace() const
{
    return SSPrintf("(node table with %d rows)", rows.size());
}



void NodeTable::Dump() const
{
	FTRACE("---------------- NodeTable -----------------\n")(rows)("\n");
}	
