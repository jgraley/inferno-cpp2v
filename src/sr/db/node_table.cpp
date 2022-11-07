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
    ASSERT( HasRow(node) )
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
            TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();
            set<const TreePtrInterface *> declared = parent_x->GetDeclared();
            return declared.count( walk_info.p_x ) > 0;
        }
        default:
        {
            return false;
        }
    }
}


void NodeTable::MonolithicClear()
{
    rows.clear();
}


void NodeTable::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.node_row_in = [=](const DBWalk::WalkInfo &walk_info)
	{
		if( !ReadArgs::use_incremental )
        {
            Row &row = rows[walk_info.xlink.GetChildX()];
            row.parents.insert( walk_info.xlink );    		

            if( IsDeclarer(walk_info) )
                row.declarers.insert( walk_info.xlink );
        }
	};
}


void NodeTable::PrepareDelete( DBWalk::Actions &actions )
{
	actions.node_row_out = [=](const DBWalk::WalkInfo &walk_info)
	{
		if( ReadArgs::use_incremental )
        {
            // ?	
        }
	};
}


void NodeTable::PrepareInsert(DBWalk::Actions &actions)
{
	actions.node_row_in = [=](const DBWalk::WalkInfo &walk_info)
	{
		if( ReadArgs::use_incremental )
        {
            Row &row = rows[walk_info.xlink.GetChildX()];
            row.parents.insert( walk_info.xlink );    

            if( IsDeclarer(walk_info) )
                row.declarers.insert( walk_info.xlink );
        }
	};
}


string NodeTable::Row::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}


string NodeTable::GetTrace() const
{
	return Trace(rows);
}
