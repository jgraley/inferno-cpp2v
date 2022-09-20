#include "tables.hpp"
#include "../sc_relation.hpp"


using namespace SR;    


Tables::Tables()
{
}


const Tables::Row &Tables::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(xlink_table);
    return xlink_table.at(xlink);
}


bool Tables::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return xlink_table.count(xlink) > 0;
}


const Tables::NodeRow &Tables::GetNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    ASSERT( HasNodeRow(node) )
          ("X tree database: no node row for ")(node)("\n")
          ("Node xlink_table: ")(node_table);
    return node_table.at(node);
}


bool Tables::HasNodeRow(TreePtr<Node> node) const
{
    ASSERT( node );
    return node_table.count(node) > 0;
}


void Tables::PopulateActions( DBWalk::Actions &actions )
{
	actions.xlink_row_in = [&](const DBWalk::WalkInfo &walk_info, 
	                           DBCommon::DepthFirstOrderedIt df_it)
	{
		// ----------------- Generate row
		Row row;        
		row.containment_context = walk_info.context;
		switch( row.containment_context )
		{
		case DBWalk::ROOT:
		{
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}	
		case DBWalk::SINGULAR:
		{
			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}
		case DBWalk::IN_SEQUENCE:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();

			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_it = walk_info.xit;        
			row.my_container_front = XLink( parent_x, &walk_info.p_xcon->front() );
			row.my_container_back = XLink( parent_x, &walk_info.p_xcon->back() );
			
			if( walk_info.xit_predecessor != walk_info.p_xcon->end() )
				row.my_sequence_predecessor = XLink( parent_x, &*walk_info.xit_predecessor );

			SequenceInterface::iterator xit_successor = walk_info.xit;
			++xit_successor;
			if( xit_successor != walk_info.p_xcon->end() )
				row.my_sequence_successor = XLink( parent_x, &*xit_successor );
			else
				row.my_sequence_successor = XLink::OffEndXLink;        
			break;
		}
		case DBWalk::IN_COLLECTION:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();

			row.parent_xlink = walk_info.parent_xlink;
			row.my_container_it = walk_info.xit;
			row.my_container_front = XLink( parent_x, &*(walk_info.p_xcon->begin()) );
			// Note: in real STL containers, one would use *(x_col->rbegin())
			row.my_container_back = XLink( parent_x, &(walk_info.p_xcon->back()) );
			break;
		}
		}
			
		row.depth_first_ordered_it = df_it;
		row.depth_first_ordinal = current_ordinal++;  
			
		// Check for badness
		if( xlink_table.count(walk_info.xlink) )
		{
			Row old_row = xlink_table.at(walk_info.xlink);
			// remember that row is incomplete because 
			// we have not been able to fill everything in yet
			if( row.parent_xlink != old_row.parent_xlink )
			{
				ASSERT(false)
					  ("Rule #217 violation or cycle: node with child should have only one parent\n")
					  ("From parents: ")(row.parent_xlink)(" and ")(old_row.parent_xlink)
					  ("\nTo child: ")(walk_info.xlink);
			}
			
			// Otherwise why did the parents not fail the check?
			ASSERTFAIL("Unknown trouble");				
		}
		
		// Keep track of the last added on the way in.
		// AddChildren() may recuse back here and update last_link.
		last_xlink = walk_info.xlink;

		// Add a row of x_tree_db
		InsertSolo( xlink_table, make_pair(walk_info.xlink, row) );
	};

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
	
	actions.xlink_row_out = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate row unwind
		// Grab last link that was added during unwind    
		xlink_table.at(walk_info.xlink).last_descendant_xlink = last_xlink;
	};
}


void Tables::PrepareFullBuild(DBWalk::Actions &actions)
{
    current_ordinal = 0;

	PopulateActions( actions );
}


void Tables::PrepareExtraXLink(DBWalk::Actions &actions)
{
	PopulateActions( actions );
}


string Tables::Row::GetTrace() const
{
    string s = "(";

    bool par = false;
    bool cont = false;
    bool idx = false;
    switch( containment_context )
    {
        case DBWalk::ROOT:
            s += "ROOT";
            break;
        case DBWalk::SINGULAR:
            s += "SINGULAR";
            par = true;
            break;
        case DBWalk::IN_SEQUENCE:
            s += "IN_SEQUENCE";
            par = cont = idx = true;
            break;
        case DBWalk::IN_COLLECTION:
            s += "IN_COLLECTION";
            par = cont = true;
            break;
    }    
    if( par )
        s += ", parent_xlink=" + Trace(parent_xlink);
    if( cont )
    {
        s += ", front=" + Trace(my_container_front);
        s += ", back=" + Trace(my_container_back);
    }
    if( idx )
        s += SSPrintf(", dfi=%d", depth_first_ordinal);
    s += ")";
    return s;
}


void Tables::NodeRow::Merge( const NodeRow &nn )
{
	parents = UnionOf( parents, nn.parents );
	declarers = UnionOf( declarers, nn.declarers );
}


string Tables::NodeRow::GetTrace() const
{
    string s = "(";
	
	s += " parents=" + Trace(parents);
	s += "declarers=" + Trace(declarers);
    
    s += ")";
    return s;
}

