#include "db_walk.hpp"

using namespace SR;    

//#define TRACE_WALK

void DBWalk::WalkTreeZone( const Action action,
                           const TreeZone &tree_zone,
                           Wind wind,
                           const DBCommon::CoreInfo *base_info )
{
    TRACE("Walking tree zone: ")(tree_zone)(" in #%u\n", tree_zone.GetTreeOrdinal());
    WalkKit kit { action, &tree_zone, tree_zone.GetTreeOrdinal(), wind, 0U };
    
    VisitBase( kit, tree_zone.GetBaseXLink(), tree_zone.GetBaseNode(), base_info );  
    ASSERT( kit.next_terminus_index == tree_zone.GetNumTerminii() )
          ("Zone has %u terminii", tree_zone.GetNumTerminii())
          (" but only visited %u", kit.next_terminus_index); // should have visited all the terminii
    TRACE("Done walking tree zone\n");
}


void DBWalk::VisitBase( const WalkKit &kit,   
						XLink base_xlink,
						TreePtr<Node> base_node,
                        const DBCommon::CoreInfo *base_info )
{
    ASSERT( base_info );
    WalkInfo walk_info = { *base_info,                       
                           base_xlink.GetTreePtrInterface(), 
                           base_xlink, 
                           base_node,
                           kit.tree_ordinal,
                           false, // at_terminus
                           true }; // at_base

    (void)VisitNode( kit, move(walk_info) );
}


void DBWalk::VisitSingular( const WalkKit &kit, 
                            const TreePtrInterface *p_x_singular, // should  be inside xlink's child
                            TreePtr<Node> node,
                            int item_ordinal )
{
    ASSERT( p_x_singular );
    ASSERT(node);
    XLink child_xlink;
    TreePtr<Node> child_node = nullptr;
    
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build a row for. 
    // Or it could be placeholder = free zone terminus.
    if( *p_x_singular )
    {
		ASSERT( node );
		child_xlink = XLink( node, p_x_singular ); // p_x_singular should be inside x
		child_node = child_xlink.GetChildTreePtr();
	}
                    
    return VisitNode( kit, 
					  { { node,
						  item_ordinal,
						  DBCommon::SINGULAR,
						  nullptr,
						  -1,                
						  ContainerInterface::iterator() },
					    p_x_singular, 
					    child_xlink, 
					    child_node,
					    kit.tree_ordinal,
					    false,
					    false } ); 
}


void DBWalk::VisitSequence( const WalkKit &kit, 
                            SequenceInterface *x_seq, // should  be inside xlink's child 
                            TreePtr<Node> node,
                            int item_ordinal )
{    
    int i=0;
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
		XLink child_xlink;
		TreePtr<Node> child_node = nullptr;
		
		if( (TreePtr<Node>)*xit ) // could be placeholder for terminus
		{
			child_xlink = XLink( node, &*xit ); // &*xit should be inside x
			child_node = child_xlink.GetChildTreePtr();
		}
			
        VisitNode(kit, 
				  { { node,
					  item_ordinal,
					  DBCommon::IN_SEQUENCE,
					  x_seq,
					  i,
					  xit },
					&*xit, 
					child_xlink, 
					child_node,
					kit.tree_ordinal,
					false,
					false } );
        i++;
    }
}


void DBWalk::VisitCollection( const WalkKit &kit, 
                              CollectionInterface *x_col, 
                              TreePtr<Node> node,
                              int item_ordinal )
{
    int i=0;
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
		XLink child_xlink;
		TreePtr<Node> child_node = nullptr;
		
		if( (TreePtr<Node>)*xit ) // could be placeholder for terminus
		{
			child_xlink = XLink( node, &*xit ); // &*xit should be inside x
			child_node = child_xlink.GetChildTreePtr();
		}
		
        VisitNode(kit, 
				  { { node,
					  item_ordinal,
					  DBCommon::IN_COLLECTION,
					  x_col,                    
					  i,
					  xit },
					&*xit, 
					child_xlink, 
					child_node,
					kit.tree_ordinal,
					false,
					false } ); // should be child_xlink's child
        i++;
    }
}


void DBWalk::VisitNode( const WalkKit &kit, 
                        WalkInfo &&walk_info ) // .x should be .xlink's child
{
#ifdef TRACE_WALK
    INDENT(".");     
#endif
    walk_info.at_terminus = (walk_info.node &&
	                         kit.tree_zone &&
							 kit.next_terminus_index < kit.tree_zone->GetNumTerminii() && // TODO store end iterator directly in kit
							 walk_info.xlink == kit.tree_zone->GetTerminusXLink(kit.next_terminus_index));
	
    if( walk_info.at_terminus )
        ++(kit.next_terminus_index);
      
    if( walk_info.node )
	{
		if( kit.wind == WIND_IN )
		{
#ifdef TRACE_WALK
			TRACE("Visiting ")
				 (walk_info.at_base?"base ":"")
				 (walk_info.at_terminus?"terminus ":"")
				 (walk_info.xlink)(" ")    
				 (walk_info.node)(" (Wind-in)\n");    
#endif
			kit.action(walk_info);
		}
				
		// Recurse into our child nodes but stop at terminii
		if( !walk_info.at_terminus )
			VisitItemise( kit, walk_info.node ); 

		if( kit.wind == WIND_OUT )
		{
#ifdef TRACE_WALK
			TRACE("Visiting ")
				 (walk_info.at_base?"base ":"")
				 (walk_info.at_terminus?"terminus ":"")
				 (walk_info.xlink)(" ")
				 (walk_info.node)(" (Wind-out)\n");    
#endif
			kit.action(walk_info);         
		}
	}
}


void DBWalk::VisitItemise( const WalkKit &kit, 
                           TreePtr<Node> node ) 
{
    ASSERT(node);
    vector< Itemiser::Element * > x_items = node->Itemise();
    for( vector< Itemiser::Element * >::size_type item_ordinal=0; item_ordinal<x_items.size(); item_ordinal++ )
    {
        Itemiser::Element *xe = x_items[item_ordinal];
        if( auto x_seq = dynamic_cast<SequenceInterface *>(xe) )
            VisitSequence( kit, x_seq, node, item_ordinal );
        else if( auto x_col = dynamic_cast<CollectionInterface *>(xe) )
            VisitCollection( kit, x_col, node, item_ordinal );
        else if( auto p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            VisitSingular( kit, p_x_singular, node, item_ordinal );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}

