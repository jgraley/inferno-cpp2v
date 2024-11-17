#include "db_walk.hpp"
#include "link_table.hpp"

using namespace SR;    

void DBWalk::Walk( const Actions *actions,
                   XLink base_xlink,
                   Context base_context,
                   const DBCommon::RootRecord *root_record, 
                   Wind wind,
                   const LinkTable::Row *base_link_row )
{
    Walk( actions, TreeZone::CreateSubtree(base_xlink), base_context, root_record, wind, base_link_row );
}


void DBWalk::Walk( const Actions *actions,
                   TreeZone zone,
                   Context base_context,
                   const DBCommon::RootRecord *root_record, 
                   Wind wind,
                   const LinkTable::Row *base_link_row )
{
    WalkKit kit { actions, zone, root_record, wind, zone.GetTerminiiBegin() };
	VisitBase( kit, base_context, base_link_row );  
	ASSERT( kit.next_terminus_it == kit.zone.GetTerminiiEnd() ); // should have visited all the terminii
}


void DBWalk::VisitBase( const WalkKit &kit,                         
                        Context context,
                        const LinkTable::Row *base_link_row )
{
    XLink base_xlink = kit.zone.GetBaseXLink();
	WalkInfo walk_info;
	if( base_link_row )
		walk_info = { base_link_row->parent_node, 
					  base_link_row->item_ordinal,
					  base_link_row->containment_context,	 
					  base_link_row->p_xcon,
					  base_link_row->container_ordinal,
					  base_link_row->my_container_it_predecessor, 
					  base_link_row->my_container_it,
					  base_xlink.GetTreePtrInterface(), 
					  base_xlink, 
					  base_xlink.GetChildTreePtr(),
					  kit.root_record,
					  false,
					  true };
	else
		walk_info = { TreePtr<Node>(), 
					  -1,
					  context,	 
					  nullptr,
					  -1,
					  ContainerInterface::iterator(), 
					  ContainerInterface::iterator(),
					  base_xlink.GetTreePtrInterface(), 
					  base_xlink, 
					  base_xlink.GetChildTreePtr(),
					  kit.root_record,
					  false,
					  true };

    VisitLink( kit, move(walk_info) );
}


void DBWalk::VisitSingular( const WalkKit &kit, 
                            const TreePtrInterface *p_x_singular, // should  be inside xlink's child
                            XLink xlink,
                            int item_ordinal )
{
    ASSERT( p_x_singular );
    
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build a row for.    
    if( !*p_x_singular )
        return;
        		
    TreePtr<Node> x = xlink.GetChildTreePtr();
    XLink child_xlink( x, p_x_singular ); // p_x_singular should be inside x
    
    VisitLink( kit, 
             { x, 
               item_ordinal,
               SINGULAR,
               nullptr,
               -1,
               ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular, 
               child_xlink, 
               child_xlink.GetChildTreePtr(),
               kit.root_record,
               false,
               false } ); 
}


void DBWalk::VisitSequence( const WalkKit &kit, 
                            SequenceInterface *x_seq, // should  be inside xlink's child 
                            XLink xlink,
                            int item_ordinal )
{
    TreePtr<Node> x = xlink.GetChildTreePtr();
    SequenceInterface::iterator xit_predecessor = x_seq->end();    
    int i=0;
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit ); // &*xit should be inside x
        VisitLink( kit, 
                 { x, 
                   item_ordinal,
                   IN_SEQUENCE,
                   x_seq,
                   i,
                   xit_predecessor,
                   xit,
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildTreePtr(),
                   kit.root_record,
                   false,
                   false } );
        xit_predecessor = xit;
        i++;
    }
}


void DBWalk::VisitCollection( const WalkKit &kit, 
                              CollectionInterface *x_col, 
                              XLink xlink,
                              int item_ordinal )
{
    TreePtr<Node> x = xlink.GetChildTreePtr();    
    int i=0;
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        VisitLink( kit, 
                 { x, 
                   item_ordinal,
                   IN_COLLECTION,
                   x_col,
                   i,
				   ContainerInterface::iterator(),
                   xit,
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildTreePtr(),
                   kit.root_record,
                   false,
				   false } ); // should be child_xlink's child
        i++;
    }
}


void DBWalk::VisitLink( const WalkKit &kit, 
                        WalkInfo &&walk_info ) // .x should be .xlink's child
{
    INDENT(".");            
    TRACE("Visiting link ")(walk_info.xlink)("\n");    
                
	walk_info.at_terminus = (kit.next_terminus_it != kit.zone.GetTerminiiEnd() &&
	                         walk_info.xlink == *(kit.next_terminus_it));
	if( walk_info.at_terminus )
		++(kit.next_terminus_it);
      
    if( kit.wind == WIND_IN )
		for( Action action : *(kit.actions) )
			action(walk_info);
            
    // Recurse into our child nodes but stop at terminii
    if( !walk_info.at_terminus )
		VisitItemise( kit, walk_info.xlink ); 

    if( kit.wind == WIND_OUT )
		for( Action action : *(kit.actions) )
			action(walk_info);         
}


void DBWalk::VisitItemise( const WalkKit &kit, 
                           XLink xlink ) // should be xlink's child
{
    ASSERT(xlink)("This probably means we're walking an incomplete tree");
    vector< Itemiser::Element * > x_items = xlink.GetChildTreePtr()->Itemise();
    for( int item_ordinal=0; item_ordinal<x_items.size(); item_ordinal++ )
    {
        Itemiser::Element *xe = x_items[item_ordinal];
        if( auto x_seq = dynamic_cast<SequenceInterface *>(xe) )
            VisitSequence( kit, x_seq, xlink, item_ordinal );
        else if( auto x_col = dynamic_cast<CollectionInterface *>(xe) )
            VisitCollection( kit, x_col, xlink, item_ordinal );
        else if( auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            VisitSingular( kit, p_x_singular, xlink, item_ordinal );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}

