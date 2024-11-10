#include "link_table.hpp"

#include "node_table.hpp"

using namespace SR;    


LinkTable::LinkTable() :
    current_root_ordinal(0)
{
}


const LinkTable::Row &LinkTable::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( HasRow(xlink) )
          ("X tree database: no row for ")(xlink)("\n")
          ("Rows: ")(rows);
    return rows.at(xlink.GetTreePtrInterface());
}


bool LinkTable::HasRow(XLink xlink) const
{
    ASSERT( xlink );
    return rows.count(xlink.GetTreePtrInterface()) > 0;
}


void LinkTable::PrepareDelete( DBWalk::Actions &actions )
{
	actions.push_back( [=](const DBWalk::WalkInfo &walk_info)
	{
		// The link row for the base does not change because its 
		// context does not change.
		if( walk_info.context != DBWalk::BASE ) 
    		EraseSolo( rows, walk_info.p_x );
	} );

    // Good practice to poison rows at terminii. Assuming walker tells us we're at a 
    // terminus, we can put in bad stuff (NULL, -1, end() etc) or maybe just a flag
    // for "parent valid". Do this in a new PoisonRow() like GenerateRow().
}


void LinkTable::PrepareInsert(DBWalk::Actions &actions)
{
	actions.push_back( [=](const DBWalk::WalkInfo &walk_info)
	{
		// The link row for the base does not change because its 
		// context does not change.
		if( walk_info.context != DBWalk::BASE ) 
    		GenerateRow(walk_info);
	} );
}


void LinkTable::GenerateRow(const DBWalk::WalkInfo &walk_info)
{
	Row row;        
	row.containment_context = walk_info.context;
	switch( row.containment_context )
	{
		case DBWalk::ROOT:
		{
			// Root ordinal filled on only for root xlinks, so that we retain
			// locality.
			row.root_ordinal = walk_info.root_record->ordinal; 
			
			// TODO wouldn't NULL ie XLink() be clearer? (and below) - no 'cause 
			// then undefineds would get into bool eval in the syms. Instead we
			// allow junk values - analysis of expressions could detect if these 
			// make it anywhere they can do harm. But we could add a new 
			// one-off link like MMAX and OffEndX? What about EmptyResult?
			row.my_container_front = walk_info.xlink; 
			row.my_container_back = walk_info.xlink;
			break;
		}	
		case DBWalk::SINGULAR:
		{
			row.parent_node = walk_info.parent_x; 
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_front = walk_info.xlink;
			row.my_container_back = walk_info.xlink;
			break;
		}
		case DBWalk::IN_SEQUENCE:
		{
			row.parent_node = walk_info.parent_x;
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_it = walk_info.xit;        
			row.my_container_front = XLink( walk_info.parent_x, &walk_info.p_xcon->front() );
			row.my_container_back = XLink( walk_info.parent_x, &walk_info.p_xcon->back() );
			row.container_ordinal = walk_info.container_ordinal;
			
			if( walk_info.xit_predecessor != walk_info.p_xcon->end() )
				row.my_sequence_predecessor = XLink( walk_info.parent_x, &*walk_info.xit_predecessor );

			SequenceInterface::iterator xit_successor = walk_info.xit;
			++xit_successor;
			if( xit_successor != walk_info.p_xcon->end() )
				row.my_sequence_successor = XLink( walk_info.parent_x, &*xit_successor );
			else
				row.my_sequence_successor = XLink::OffEndXLink;        
			break;
		}
		case DBWalk::IN_COLLECTION:
		{
			row.parent_node = walk_info.parent_x;
			row.item_ordinal = walk_info.item_ordinal;
			row.my_container_it = walk_info.xit;
			row.my_container_front = XLink( walk_info.parent_x, &*(walk_info.p_xcon->begin()) );
			// Note: in real STL containers, one would use *(x_col->rbegin())
			row.my_container_back = XLink( walk_info.parent_x, &(walk_info.p_xcon->back()) );
			row.container_ordinal = walk_info.container_ordinal;
			break;
		}
        default:
            ASSERTFAIL(); // could be UNKNOWN but that's not allowed 
            // TODO we have to add support for this. Assuming it's called BASE, we'll need a base_context 
            // object which could be returned from Delete() or a new getter method. Or the whole
            // Delete->Insert cycle could be made a 2-beat transaction with its own classes. Delete()
            // or whatever would get it from the current XLink row at the base location before deletion.
            
            // We also have to fix child node entries at terminii. Assuming walker tells us we're at a 
            // terminus, we can look at children ourself (walker won't visit them) and fix up their rows.
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
    s += SSPrintf(", bo=%d", root_ordinal);
    s += ")";
    return s;
}


string LinkTable::GetTrace() const
{
	return SSPrintf("Link Table with %d rows", rows.size());
}
