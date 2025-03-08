#include "db_walk.hpp"

using namespace SR;    

void DBWalk::WalkTree( const Actions *actions,
                       XLink root_xlink,
                       const DBCommon::TreeOrdinal tree_ordinal, 
                       Wind wind )
{
	ASSERT(root_xlink);
	const DBCommon::CoreInfo root_info = { TreePtr<Node>(), 			          
										   -1,
										   DBCommon::ROOT,  				
										   nullptr,
										   -1,	 	  					  					  
										   ContainerInterface::iterator() };
										   
	const TreeZone zone = TreeZone::CreateSubtree(root_xlink);
    WalkKit kit { actions, zone, tree_ordinal, wind, zone.GetTerminiiBegin() };
	VisitBase( kit, &root_info );  
}


void DBWalk::WalkSubtree( const Actions *actions,
						  XLink base_xlink,
						  const DBCommon::TreeOrdinal tree_ordinal, 
						  Wind wind,
						  const DBCommon::CoreInfo *base_info )
{
	ASSERT( base_info );
    WalkZone( actions, TreeZone::CreateSubtree(base_xlink), tree_ordinal, wind, base_info );
}


void DBWalk::WalkZone( const Actions *actions,
					   const TreeZone zone,
                       const DBCommon::TreeOrdinal tree_ordinal, 
                       Wind wind,
                       const DBCommon::CoreInfo *base_info )
{
	ASSERT( base_info );
    WalkKit kit { actions, zone, tree_ordinal, wind, zone.GetTerminiiBegin() };
	VisitBase( kit, base_info );  
	ASSERT( kit.next_terminus_it == zone.GetTerminiiEnd() ); // should have visited all the terminii
}


void DBWalk::VisitBase( const WalkKit &kit,                         
                        const DBCommon::CoreInfo *base_info )
{
	ASSERT( base_info );

    XLink base_xlink = kit.zone.GetBaseXLink();
	WalkInfo walk_info = { *base_info, 			          
						   base_xlink.GetTreePtrInterface(), 
						   base_xlink, 
						   base_xlink.GetChildTreePtr(),
						   kit.tree_ordinal,
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
             { { x,
				 item_ordinal,
				 DBCommon::SINGULAR,
				 nullptr,
				 -1,                
                 ContainerInterface::iterator() },
               p_x_singular, 
               child_xlink, 
               child_xlink.GetChildTreePtr(),
               kit.tree_ordinal,
               false,
               false } ); 
}


void DBWalk::VisitSequence( const WalkKit &kit, 
                            SequenceInterface *x_seq, // should  be inside xlink's child 
                            XLink xlink,
                            int item_ordinal )
{
    TreePtr<Node> x = xlink.GetChildTreePtr();
    int i=0;
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit ); // &*xit should be inside x
        VisitLink( kit, 
                 { { x,
					 item_ordinal,
					 DBCommon::IN_SEQUENCE,
					 x_seq,
                     i,
                     xit },
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildTreePtr(),
                   kit.tree_ordinal,
                   false,
                   false } );
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
                 { { x,
					 item_ordinal,
					 DBCommon::IN_COLLECTION,
					 x_col,                    
                     i,
                     xit },
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildTreePtr(),
                   kit.tree_ordinal,
                   false,
				   false } ); // should be child_xlink's child
        i++;
    }
}


void DBWalk::VisitLink( const WalkKit &kit, 
                        WalkInfo &&walk_info ) // .x should be .xlink's child
{
    INDENT(".");            
                
	walk_info.at_terminus = (kit.next_terminus_it != kit.zone.GetTerminiiEnd() &&
	                         walk_info.xlink == *(kit.next_terminus_it));
	if( walk_info.at_terminus )
		++(kit.next_terminus_it);
	ASSERT( !(walk_info.at_terminus && walk_info.at_base) );
      
    if( kit.wind == WIND_IN )
    {
	    TRACE("Visiting ")(walk_info.at_base?"base ":"")(walk_info.at_terminus?"terminus ":"")(walk_info.xlink)(" (Wind-in)\n");    
		for( Action action : *(kit.actions) )
			action(walk_info);
	}
            
    // Recurse into our child nodes but stop at terminii
    if( !walk_info.at_terminus )
		VisitItemise( kit, walk_info.xlink ); 

    if( kit.wind == WIND_OUT )
    {
	    TRACE("Visiting ")(walk_info.at_base?"base ":"")(walk_info.at_terminus?"terminus ":"")(walk_info.xlink)(" (Wind-out)\n");    		
		for( Action action : *(kit.actions) )
			action(walk_info);         
	}
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

