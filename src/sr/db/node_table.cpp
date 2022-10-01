#include "node_table.hpp"
#include "sc_relation.hpp"


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


void NodeTable::MonolithicClear()
{
    rows.clear();
}


void NodeTable::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.node_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate node row
		Row row;
		switch( walk_info.context )
		{
            case DBWalk::ROOT:
            {
                break;
            }	
            case DBWalk::SINGULAR:
            case DBWalk::IN_SEQUENCE:
            case DBWalk::IN_COLLECTION:
            {
                TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();
                set<const TreePtrInterface *> declared = parent_x->GetDeclared();
                if( declared.count( walk_info.p_x ) > 0 )
                    row.declarers.insert( walk_info.xlink );
                break;
            }
		}
		row.parents.insert( walk_info.xlink );    

		// Merge in the node row
		rows[walk_info.xlink.GetChildX()].Merge( row );			
	};
}


void NodeTable::PrepareDelete( DBWalk::Actions &actions )
{
}


void NodeTable::PrepareInsert(DBWalk::Actions &actions)
{
}


void NodeTable::Row::Merge( const Row &nn )
{
	parents = UnionOf( parents, nn.parents );
	declarers = UnionOf( declarers, nn.declarers );
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
