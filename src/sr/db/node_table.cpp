#include "node_table.hpp"
#include "sc_relation.hpp"

#include "common/read_args.hpp"

using namespace SR;    


NodeTable::NodeTable()
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


bool NodeTable::IsDeclarer(const DBWalk::WalkInfo &walk_info) const
{
    switch( walk_info.core.context_type )
    {
        case DBCommon::SINGULAR:
        case DBCommon::IN_SEQUENCE:
        case DBCommon::IN_COLLECTION:
        {
            set<const TreePtrInterface *> declared = walk_info.core.parent_node->GetDeclared();
            return declared.count( walk_info.p_tree_ptr_interface ) > 0;
        }
        default:
        {
            return false;
        }
    }
}


void NodeTable::Insert(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{     
	DBWalk::Actions actions;
	actions.push_back( bind(&NodeTable::InsertAction, this, placeholders::_1) );
	db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal(-1), DBWalk::WIND_IN, base_info );
}


void NodeTable::Delete(TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{
	DBWalk::Actions actions;
	actions.push_back( bind(&NodeTable::DeleteAction, this, placeholders::_1) );
	db_walker.WalkTreeZone( &actions, zone, DBCommon::TreeOrdinal(-1), DBWalk::WIND_OUT, base_info );
}


void NodeTable::InsertAction(const DBWalk::WalkInfo &walk_info)
{
	// Create if not already there
	TRACE("NODE TABLE weakly inserts row for: ")(walk_info.node)("\n");
	Row &row = rows[walk_info.node];
	
	InsertSolo( row.incoming_xlinks, walk_info.xlink );            
	if( IsDeclarer(walk_info) )
		InsertSolo( row.declaring_xlinks, walk_info.xlink );
}


void NodeTable::DeleteAction(const DBWalk::WalkInfo &walk_info)
{
	// Should already be there
	Row &row = rows.at(walk_info.node);
	
	EraseSolo( row.incoming_xlinks, walk_info.xlink );            
	if( IsDeclarer(walk_info) )
		EraseSolo( row.declaring_xlinks, walk_info.xlink );
		
	if( !row.incoming_xlinks.empty() )
	{
		TRACE("NODE TABLE does not delete row due %u>0 remaining incoming: ", row.incoming_xlinks.size())(walk_info.node)("\n");
		return;
	}
	
	TRACE("NODE TABLE deletes row for: ")(walk_info.node)("\n");
	EraseSolo( rows, walk_info.node );
}


string NodeTable::Row::GetTrace() const
{
    string s = "(";
    
    s += " incoming=" + Trace(incoming_xlinks);
    s += " declaring=" + Trace(declaring_xlinks);
    
    s += ")";
    return s;
}


vector<TreePtr<Node>> NodeTable::GetNodeDomainAsVector() const
{
    vector<TreePtr<Node>> v;
    for( auto p : rows )
        v.push_back(p.first);
    return v;
}


string NodeTable::GetTrace() const
{
    return SSPrintf("(node table with %d rows)", rows.size());
}



void NodeTable::Dump() const
{
	FTRACE("---------------- NodeTable -----------------\n")(rows)("\n");
}	
