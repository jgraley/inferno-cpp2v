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


DBWalk::Action NodeTable::GetInsertAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{
        // Create if not already there
        Row &row = rows[walk_info.x];
        
        InsertSolo( row.incoming_xlinks, walk_info.xlink );    		
        if( IsDeclarer(walk_info) )
            InsertSolo( row.declaring_xlinks, walk_info.xlink );
	};
}


DBWalk::Action NodeTable::GetDeleteAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{
        // Should already be there
        Row &row = rows.at(walk_info.x);
        
        EraseSolo( row.incoming_xlinks, walk_info.xlink );    		
        if( IsDeclarer(walk_info) )
            EraseSolo( row.declaring_xlinks, walk_info.xlink );
            
        if( row.incoming_xlinks.empty() )
            EraseSolo( rows, walk_info.x );
	};
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
	return Trace(rows);
}
