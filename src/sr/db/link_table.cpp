#include "link_table.hpp"

#include "node_table.hpp"

using namespace SR;    

//#define INCREMENTAL


LinkTable::LinkTable() :
    current_base_ordinal(0)
{
}


const LinkTable::Row &LinkTable::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(rows);
    return rows.at(xlink.GetXPtr());
}


bool LinkTable::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return rows.count(xlink.GetXPtr()) > 0;
}


void LinkTable::MonolithicClear()
{
#ifndef INCREMENTAL
    rows.clear();
    // We do not reset current_base_ordinal, but just let it spin. New
    // domain extras are always added to the end of the ordering and 
    // we never need to renumber.
#endif
}


void LinkTable::PrepareMonolithicBuild(DBWalk::Actions &actions)
{
#ifndef INCREMENTAL
	actions.link_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		ASSERT( walk_info.context != DBWalk::UNKNOWN );
    	GenerateRow(walk_info);
	};
#endif
}


void LinkTable::PrepareDelete( DBWalk::Actions &actions )
{
#ifdef INCREMENTAL
	actions.link_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		if( walk_info.context != DBWalk::UNKNOWN )
    		rows.erase( walk_info.p_x );
	};
#endif
}


void LinkTable::PrepareInsert(DBWalk::Actions &actions)
{
#ifdef INCREMENTAL
	actions.link_row_in = [&](const DBWalk::WalkInfo &walk_info)
	{
		if( walk_info.context != DBWalk::UNKNOWN )
    		GenerateRow(walk_info);
	};
#endif
}


void LinkTable::GenerateRow(const DBWalk::WalkInfo &walk_info)
{
	Row row;        
	row.containment_context = walk_info.context;
	switch( row.containment_context )
	{
		case DBWalk::ROOT:
		{
			// Base ordinal filled on only for base xlinks, so that we retain
			// locality.
			row.base_ordinal = current_base_ordinal++;
			
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}	
		case DBWalk::SINGULAR:
		{
			row.parent_node = walk_info.parent_xlink.GetChildX();
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}
		case DBWalk::IN_SEQUENCE:
		{
			TreePtr<Node> parent_x = walk_info.parent_xlink.GetChildX();

			row.parent_node = walk_info.parent_xlink.GetChildX();
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_it = walk_info.xit;        
			row.my_container_front = XLink( parent_x, &walk_info.p_xcon->front() );
			row.my_container_back = XLink( parent_x, &walk_info.p_xcon->back() );
			row.container_ordinal = walk_info.container_ordinal;
			
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

			row.parent_node = walk_info.parent_xlink.GetChildX();
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_it = walk_info.xit;
			row.my_container_front = XLink( parent_x, &*(walk_info.p_xcon->begin()) );
			// Note: in real STL containers, one would use *(x_col->rbegin())
			row.my_container_back = XLink( parent_x, &(walk_info.p_xcon->back()) );
			row.container_ordinal = walk_info.container_ordinal;
			break;
		}
	}
		
	// Check for badness
	if( rows.count(walk_info.xlink.GetXPtr()) )
	{
		// Otherwise why did the parents not fail the check?
		ASSERTFAIL("Unknown trouble");				
	}

	// Add a row of x_tree_db
	InsertSolo( rows, make_pair(walk_info.p_x, row) );
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
        s += ", parent_node=" + Trace(parent_node);
    if( cont )
    {
        s += ", front=" + Trace(my_container_front);
        s += ", back=" + Trace(my_container_back);
    }
    s += SSPrintf(", co=%d", container_ordinal);
    s += SSPrintf(", bo=%d", base_ordinal);
    s += ")";
    return s;
}


string LinkTable::GetTrace() const
{
	return Trace(rows);
}
