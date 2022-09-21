#include "node_table.hpp"
#include "../sc_relation.hpp"


using namespace SR;    


NodeTable::NodeTable()
{
}


const NodeTable::NodeRow &NodeTable::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasNodeRow(node) )
          ("X tree database: no node row for ")(node)("\n")
          ("Node xlink_table: ")(node_table);
    return node_table.at(node);
}


bool NodeTable::HasNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return node_table.count(node) > 0;
}


void NodeTable::PopulateActions( DBWalk::Actions &actions )
{
	actions.node_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate node row
		NodeRow node_row;
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
				node_row.declarers.insert( walk_info.xlink );
			break;
		}
		}
		node_row.parents.insert( walk_info.xlink );    

		// Merge in the node row
		node_table[walk_info.xlink.GetChildX()].Merge( node_row );			
	};
}


void NodeTable::PrepareFullBuild(DBWalk::Actions &actions)
{
	PopulateActions( actions );
}


void NodeTable::PrepareExtraXLink(DBWalk::Actions &actions)
{
	PopulateActions( actions );
}


void NodeTable::NodeRow::Merge( const NodeRow &nn )
{
	parents = UnionOf( parents, nn.parents );
	declarers = UnionOf( declarers, nn.declarers );
}


string NodeTable::NodeRow::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}

