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
 
 
void LinkTable::Insert(DBCommon::TreeOrdinal tree_ordinal, TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{     
	DBWalk::Actions actions;
	actions.push_back( bind(&LinkTable::InsertAction, this, placeholders::_1) );
	db_walker.WalkTreeZone( &actions, zone, tree_ordinal, DBWalk::WIND_IN, base_info );
}


void LinkTable::Delete(DBCommon::TreeOrdinal tree_ordinal, TreeZone &zone, const DBCommon::CoreInfo *base_info, bool do_intrinsics)
{
	DBWalk::Actions actions;
	actions.push_back( bind(&LinkTable::DeleteAction, this, placeholders::_1) );
	db_walker.WalkTreeZone( &actions, zone, tree_ordinal, DBWalk::WIND_OUT, base_info );
}
 
 
void LinkTable::InsertAction(const DBWalk::WalkInfo &walk_info)
{
    GenerateRow(walk_info.xlink, walk_info.tree_ordinal, &walk_info.core);
}


void LinkTable::DeleteAction(const DBWalk::WalkInfo &walk_info)
{
    EraseSolo( rows, walk_info.xlink );
    // Good practice to poison rows at terminii. Assuming walker tells us we're at a 
    // terminus, we can put in bad stuff (NULL, -1, end() etc) or maybe just a flag
    // for "parent valid". Do this in a new PoisonRow() like GenerateRow().
}


void LinkTable::GenerateRow(XLink xlink, DBCommon::TreeOrdinal tree_ordinal, const DBCommon::CoreInfo *core_info)
{
    Row row;        
    *(DBCommon::CoreInfo *)(&row) = *core_info;
    switch( row.context_type )
    {
        case DBCommon::ROOT:
        {
            // Root ordinal filled on only for root xlinks, so that we retain
            // locality.
            row.tree_ordinal = tree_ordinal; 
            
            // TODO wouldn't NULL ie XLink() be clearer? (and below) - no 'cause 
            // then undefineds would get into bool eval in the syms. Instead we
            // allow junk values - analysis of expressions could detect if these 
            // make it anywhere they can do harm. But we could add a new 
            // one-off link like MMAX and OffEndX? What about EmptyResult?
            row.container_front = xlink; 
            row.container_back = xlink;
            break;
        }    
        case DBCommon::SINGULAR:
        {
            row.container_front = xlink;
            row.container_back = xlink;
            break;
        }
        case DBCommon::IN_SEQUENCE:
        {  
            row.container_front = XLink( core_info->parent_node, &core_info->p_container->front() );
            row.container_back = XLink( core_info->parent_node, &core_info->p_container->back() );
            
            SequenceInterface::iterator xit_predecessor = core_info->container_it;
            if( xit_predecessor != row.p_container->begin() )
            {
                --xit_predecessor;
                row.sequence_predecessor = XLink( core_info->parent_node, &*xit_predecessor );
            }
            else
                row.sequence_predecessor = XLink::OffEndXLink;        

            SequenceInterface::iterator xit_successor = core_info->container_it;
            ++xit_successor;
            if( xit_successor != row.p_container->end() )
                row.sequence_successor = XLink( core_info->parent_node, &*xit_successor );
            else
                row.sequence_successor = XLink::OffEndXLink;        
            break;
        }
        case DBCommon::IN_COLLECTION:
        {
            row.container_front = XLink( core_info->parent_node, &*(core_info->p_container->begin()) );
            // Note: in real STL containers, one would use *(x_col->rbegin())
            row.container_back = XLink( core_info->parent_node, &(core_info->p_container->back()) );
            break;
        }
    }

    // Add a row of x_tree_db
    InsertSolo( rows, make_pair(xlink, row) );
}


LinkTable::RAIISuspendForSwap::RAIISuspendForSwap(LinkTable *link_table_,
                                                  DBCommon::TreeOrdinal tree_ordinal1_, TreeZone &zone1_, 
												  DBCommon::TreeOrdinal tree_ordinal2_, TreeZone &zone2_ ) :
	DBCommon::RAIISuspendForSwap( tree_ordinal1_, zone1_, tree_ordinal2_, zone2_ ),
	link_table( *link_table_ ),
	rows( link_table.rows )
{	
	// Erase zone bases
	mybase_info1 = rows.at( zone1.GetBaseXLink() );
	EraseSolo( rows, zone1.GetBaseXLink() );
	
	mybase_info2 = rows.at( zone2.GetBaseXLink() );
	EraseSolo( rows, zone2.GetBaseXLink() );
	
	// Erase terminii, storing core info for all of them
	for( XLink terminus : zone1.GetTerminusXLinks() )
	{
		terminus_info1.push( rows.at(terminus) );
		EraseSolo( rows, terminus );	
	}	
	for( XLink terminus : zone2.GetTerminusXLinks() )
	{
		terminus_info2.push( rows.at(terminus) );
		EraseSolo( rows, terminus );	
	}
}


LinkTable::RAIISuspendForSwap::~RAIISuspendForSwap()
{
	// Regenerate base rows using info supplied to us TODO do that here?
	link_table.GenerateRow(zone1.GetBaseXLink(), tree_ordinal1, &mybase_info1);
	link_table.GenerateRow(zone2.GetBaseXLink(), tree_ordinal2, &mybase_info2);

	// Stored core info relates to interipr of the zones, and should be
	// swapped alongside the zones themselves
	swap( terminus_info1, terminus_info2 );

	// Regenerate terminus rows using a mixture of supplied and stored info
	for( XLink terminus : zone1.GetTerminusXLinks() )
	{
		link_table.GenerateRow(terminus, tree_ordinal1, &terminus_info1.front());
		terminus_info1.pop();
	}
	for( XLink terminus : zone2.GetTerminusXLinks() )
	{
		link_table.GenerateRow(terminus, tree_ordinal2, &terminus_info2.front());
		terminus_info2.pop();
	}
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


void LinkTable::Dump() const
{
	FTRACE("---------------- LinkTable -----------------\n")(rows)("\n");
}	
	
