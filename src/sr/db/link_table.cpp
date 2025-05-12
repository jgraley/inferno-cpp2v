#include "link_table.hpp"

#include "node_table.hpp"

using namespace SR;    


LinkTable::LinkTable()
{
}


const LinkTable::Row &LinkTable::GetRow(XLink xlink) const
{
    ASSERT( xlink );
    ASSERT( xlink.GetChildTreePtr() );
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

 
const DBCommon::CoreInfo &LinkTable::GetCoreInfo(XLink xlink) const
{
    return *(DBCommon::CoreInfo *)&(GetRow(xlink));    
}
 
 
void LinkTable::InsertAction(const DBWalk::WalkInfo &walk_info)
{
    GenerateRow(walk_info);
}


void LinkTable::DeleteAction(const DBWalk::WalkInfo &walk_info)
{
    EraseSolo( rows, walk_info.xlink );
    // Good practice to poison rows at terminii. Assuming walker tells us we're at a 
    // terminus, we can put in bad stuff (NULL, -1, end() etc) or maybe just a flag
    // for "parent valid". Do this in a new PoisonRow() like GenerateRow().
}


void LinkTable::GenerateRow(const DBWalk::WalkInfo &walk_info)
{
    Row row;        
    *(DBCommon::CoreInfo *)(&row) = walk_info.core;
    switch( row.context_type )
    {
        case DBCommon::ROOT:
        {
            // Root ordinal filled on only for root xlinks, so that we retain
            // locality.
            row.tree_ordinal = walk_info.tree_ordinal; 
            
            // TODO wouldn't NULL ie XLink() be clearer? (and below) - no 'cause 
            // then undefineds would get into bool eval in the syms. Instead we
            // allow junk values - analysis of expressions could detect if these 
            // make it anywhere they can do harm. But we could add a new 
            // one-off link like MMAX and OffEndX? What about EmptyResult?
            row.container_front = walk_info.xlink; 
            row.container_back = walk_info.xlink;
            break;
        }    
        case DBCommon::SINGULAR:
        {
            row.container_front = walk_info.xlink;
            row.container_back = walk_info.xlink;
            break;
        }
        case DBCommon::IN_SEQUENCE:
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
        case DBCommon::IN_COLLECTION:
        {
            row.container_front = XLink( walk_info.core.parent_node, &*(walk_info.core.p_container->begin()) );
            // Note: in real STL containers, one would use *(x_col->rbegin())
            row.container_back = XLink( walk_info.core.parent_node, &(walk_info.core.p_container->back()) );
            break;
        }
        case DBCommon::FREE_BASE:
        {
            // No way to reach parent so do nothing
            return;
        }
    }

    // Add a row of x_tree_db
    InsertSolo( rows, make_pair(walk_info.xlink, row) );
}


string LinkTable::Row::GetTrace() const
{
    string s = "(cc=";

    bool par = false;
    bool cont = false;
    switch( context_type )
    {
        case DBCommon::ROOT:
            s += "ROOT";
            break;
        case DBCommon::SINGULAR:
            s += "SINGULAR";
            par = true;
            break;
        case DBCommon::IN_SEQUENCE:
            s += "IN_SEQUENCE";
            par = cont = true;
            break;
        case DBCommon::IN_COLLECTION:
            s += "IN_COLLECTION";
            par = cont = true;
            break;
        case DBCommon::FREE_BASE:
			s += "FREE_BASE";
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
    s += SSPrintf(", to=%d", tree_ordinal);
    s += ")";
    return s;
}


vector<XLink> LinkTable::GetXLinkDomainAsVector() const
{
    vector<XLink> v;
    for( auto p : rows )
        v.push_back(p.first);
    return v;
}


string LinkTable::GetTrace() const
{
    return SSPrintf("(link table with %d rows)", rows.size());
}
