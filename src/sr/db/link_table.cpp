#include "link_table.hpp"


using namespace SR;    


LinkTable::LinkTable()
{
}


const LinkTable::Row &LinkTable::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(rows);
    return rows.at(xlink);
}


bool LinkTable::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return rows.count(xlink) > 0;
}


void LinkTable::MonolithicClear()
{
    rows.clear();
    current_ordinal = 0;
}


void LinkTable::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
	actions.link_row_in = [&](const DBWalk::WalkInfo &walk_info)
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
			
		row.depth_first_ordinal = current_ordinal++;  
			
		// Check for badness
		if( rows.count(walk_info.xlink) )
		{
			Row old_row = rows.at(walk_info.xlink);
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
		// VisitItemise() may recuse back here and update last_link.
		last_xlink = walk_info.xlink;

		// Add a row of x_tree_db
		InsertSolo( rows, make_pair(walk_info.xlink, row) );
	};
	
	actions.link_row_out = [&](const DBWalk::WalkInfo &walk_info)
	{
		// ----------------- Generate row unwind
		// Grab last link that was added during unwind    
		rows.at(walk_info.xlink).last_descendant_xlink = last_xlink;
	};
}


void LinkTable::PrepareDelete( DBWalk::Actions &actions )
{
}


void LinkTable::PrepareInsert(DBWalk::Actions &actions)
{
}


string LinkTable::Row::GetTrace() const
{
    string s = "(cc=";

    bool par = false;
    bool cont = false;
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
            par = cont = true;
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
    s += SSPrintf(", dfo=%d", depth_first_ordinal);
    s += ")";
    return s;
}


string LinkTable::GetTrace() const
{
	return Trace(rows);
}
