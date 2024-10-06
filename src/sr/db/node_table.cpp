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
          ("Node xlink_table: ")(rows);
    return rows.at(node);
}


bool NodeTable::HasRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return rows.count(node) > 0;
}


bool NodeTable::IsDeclarer(const DBWalk::WalkInfo &walk_info) const
{
    switch( walk_info.context )
    {
        case DBWalk::SINGULAR:
        case DBWalk::IN_SEQUENCE:
        case DBWalk::IN_COLLECTION:
        {
            set<const TreePtrInterface *> declared = walk_info.parent_x->GetDeclared();
            return declared.count( walk_info.p_x ) > 0;
        }
        default:
        {
            return false;
        }
    }
}


void NodeTable::PrepareDelete( DBWalk::Actions &actions )
{
	actions.node_row_out = [=](const DBWalk::WalkInfo &walk_info)
	{
        // Should already be there
        Row &row = rows.at(walk_info.x);
        
        EraseSolo( row.parents, walk_info.xlink );    		
        if( IsDeclarer(walk_info) )
            EraseSolo( row.declarers, walk_info.xlink );
            
        if( row.parents.empty() )
            EraseSolo( rows, walk_info.x );
	};
}


void NodeTable::PrepareInsert(DBWalk::Actions &actions)
{
	actions.node_row_in = [=](const DBWalk::WalkInfo &walk_info)
	{
        // Create if not already there
        Row &row = rows[walk_info.x];
        
        InsertSolo( row.parents, walk_info.xlink );    		
        if( IsDeclarer(walk_info) )
            InsertSolo( row.declarers, walk_info.xlink );
	};
}


string NodeTable::Row::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += " declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}


string NodeTable::GetTrace() const
{
	return Trace(rows);
}
