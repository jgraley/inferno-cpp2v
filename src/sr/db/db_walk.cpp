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
                                           
    WalkKit kit { actions, nullptr, tree_ordinal, wind, 0U };
    VisitBase( kit, root_xlink, &root_info );  
}


void DBWalk::WalkSubtree( const Actions *actions,
                          XLink base_xlink,
                          const DBCommon::TreeOrdinal tree_ordinal, 
                          Wind wind,
                          const DBCommon::CoreInfo *base_info )
{
    ASSERT( base_info );
	WalkKit kit { actions, nullptr, tree_ordinal, wind, 0U };
    VisitBase( kit, base_xlink, base_info );
}


void DBWalk::WalkTreeZone( const Actions *actions,
                           const TreeZone *zone,
                           const DBCommon::TreeOrdinal tree_ordinal, 
                           Wind wind,
                           const DBCommon::CoreInfo *base_info )
{
    //FTRACE("Walking zone: ")(*zone)("\n");
    ASSERT( base_info );
    WalkKit kit { actions, zone, tree_ordinal, wind, 0U };
    VisitBase( kit, zone->GetBaseXLink(), base_info );  
    ASSERT( kit.next_terminus_index == zone->GetNumTerminii() )
          ("Zone has %d terminii", zone->GetNumTerminii())
          (kit.next_terminus_index==0?" (no terminii visited)":""); // should have visited all the terminii
}


void DBWalk::WalkFreeZone( const Actions *actions,
                           const FreeZone *zone,
                           Wind wind )
{
	ASSERTFAIL(); // Broken!
	
    // Behaviour for free zones:
    // - Context is FREE_BASE for base of zone, walk_info similar to ROOT
    // - We do not visit terminii, so at_terminus is always false 
 
    //FTRACE("Walking zone: ")(*zone)("\n");
    const DBCommon::CoreInfo base_info = { TreePtr<Node>(),                       
                                           -1,
                                           DBCommon::FREE_BASE,                  
                                           nullptr,
                                           -1,                                                       
                                           ContainerInterface::iterator() };
    WalkKit kit { actions, nullptr, (SR::DBCommon::TreeOrdinal)(-1), wind, 0U };
    VisitBase( kit, XLink(), &base_info );  
}


void DBWalk::VisitBase( const WalkKit &kit,   
						XLink base_xlink,
                        const DBCommon::CoreInfo *base_info )
{
    ASSERT( base_info );
    WalkInfo walk_info = { *base_info,                       
                           base_xlink.GetTreePtrInterface(), 
                           base_xlink, 
                           base_xlink.GetChildTreePtr(),
                           kit.tree_ordinal,
                           false, // at_terminus
                           true }; // at_base

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
    walk_info.at_terminus = (kit.zone &&
                             kit.next_terminus_index < kit.zone->GetNumTerminii() && // TODO store end iterator directly in kit
                             walk_info.xlink == kit.zone->GetTerminusXLink(kit.next_terminus_index));
    if( walk_info.at_terminus )
        ++(kit.next_terminus_index);
      
    if( kit.wind == WIND_IN )
    {
        //FTRACE("Visiting ")(walk_info.at_base?"base ":"")(walk_info.at_terminus?"terminus ":"")(walk_info.xlink)(" (Wind-in)\n");    
        for( Action action : *(kit.actions) )
            action(walk_info);
    }
            
    // Recurse into our child nodes but stop at terminii
    if( !walk_info.at_terminus )
        VisitItemise( kit, walk_info.xlink ); 

    if( kit.wind == WIND_OUT )
    {
        //FTRACE("Visiting ")(walk_info.at_base?"base ":"")(walk_info.at_terminus?"terminus ":"")(walk_info.xlink)(" (Wind-out)\n");            
        for( Action action : *(kit.actions) )
            action(walk_info);         
    }
}


void DBWalk::VisitItemise( const WalkKit &kit, 
                           XLink xlink ) // should be xlink's child
{
    ASSERT(xlink)("This probably means we're walking an incomplete tree");
    vector< Itemiser::Element * > x_items = xlink.GetChildTreePtr()->Itemise();
    for( vector< Itemiser::Element * >::size_type item_ordinal=0; item_ordinal<x_items.size(); item_ordinal++ )
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

