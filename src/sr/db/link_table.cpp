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

 
const DBWalk::CoreInfo &LinkTable::GetCoreInfo(XLink xlink) const
{
    return *(DBWalk::CoreInfo *)&(GetRow(xlink));	
}
 
 
DBWalk::Action LinkTable::GetInsertAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{
   		GenerateRow(walk_info);
	};
}


DBWalk::Action LinkTable::GetDeleteAction()
{
	return [=](const DBWalk::WalkInfo &walk_info)
	{
   		EraseSolo( rows, walk_info.p_tree_ptr_interface );
	};

    // Good practice to poison rows at terminii. Assuming walker tells us we're at a 
    // terminus, we can put in bad stuff (NULL, -1, end() etc) or maybe just a flag
    // for "parent valid". Do this in a new PoisonRow() like GenerateRow().
}


void LinkTable::GenerateRow(const DBWalk::WalkInfo &walk_info)
{
	Row row;        
	*(DBWalk::CoreInfo *)(&row) = walk_info.core;
	switch( row.context_type )
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
			row.container_front = walk_info.xlink; 
			row.container_back = walk_info.xlink;
			break;
		}	
		case DBWalk::SINGULAR:
		{
			row.container_front = walk_info.xlink;
			row.container_back = walk_info.xlink;
			break;
		}
		case DBWalk::IN_SEQUENCE:
		{  
			row.container_front = XLink( walk_info.core.parent_node, &walk_info.core.p_container->front() );
			row.container_back = XLink( walk_info.core.parent_node, &walk_info.core.p_container->back() );
			
			SequenceInterface::iterator xit_predecessor = walk_info.core.container_it;
			if( xit_predecessor != walk_info.core.p_container->begin() )
			{
			    --xit_predecessor;
				row.sequence_predecessor = XLink( walk_info.core.parent_node, &*xit_predecessor );
			}
			else
				row.sequence_predecessor = XLink::OffEndXLink;        

			SequenceInterface::iterator xit_successor = walk_info.core.container_it;
			++xit_successor;
			if( xit_successor != walk_info.core.p_container->end() )
				row.sequence_successor = XLink( walk_info.core.parent_node, &*xit_successor );
			else
				row.sequence_successor = XLink::OffEndXLink;        
			break;
		}
		case DBWalk::IN_COLLECTION:
		{
			row.container_front = XLink( walk_info.core.parent_node, &*(walk_info.core.p_container->begin()) );
			// Note: in real STL containers, one would use *(x_col->rbegin())
			row.container_back = XLink( walk_info.core.parent_node, &(walk_info.core.p_container->back()) );
			break;
		}
	}

	// Add a row of x_tree_db
	InsertSolo( rows, make_pair(walk_info.p_tree_ptr_interface, row) );
}


unique_ptr<Mutator> LinkTable::GetMutator(XLink xlink) const
{
	const LinkTable::Row &row = GetRow(xlink);
	
	switch( row.context_type )
	{
		case DBWalk::ROOT:
		{
			// We're still const casting here, TODO
			const TreePtrInterface *const_tpi = xlink.GetTreePtrInterface();
			TreePtrInterface *tpi = const_cast<TreePtrInterface *>(const_tpi);
			return make_unique<SingularMutator>( row.parent_node, tpi );
		}	
		case DBWalk::SINGULAR:
		{
			vector< Itemiser::Element * > x_items = row.parent_node->Itemise();
			Itemiser::Element *xe = x_items[row.item_ordinal];		
			auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe);
			ASSERT( p_x_singular );
			return make_unique<SingularMutator>( row.parent_node, p_x_singular );
		}
		case DBWalk::IN_SEQUENCE:
		case DBWalk::IN_COLLECTION: 
		{
			// COLLECTION is the motivating case: its elements are const, so we neet Mutate() to change them
			return make_unique<ContainerMutator>( row.parent_node, row.p_container, row.container_it );			
		}
	}	
	ASSERTFAIL();
}


string LinkTable::Row::GetTrace() const
{
    string s = "(cc=";

    bool par = false;
    bool cont = false;
    switch( context_type )
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
        s += ", front=" + Trace(container_front);
        s += ", back=" + Trace(container_back);
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
