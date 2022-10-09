#include "db_walk.hpp"

using namespace SR;    

void DBWalk::FullWalk( const Actions *actions,
                       XLink root_xlink )
{
    WalkKit kit { actions, REQUIRE_SOLO };
    
	VisitBase( kit, root_xlink );
    
    VisitBase( kit, XLink::MMAX_Link );
    VisitBase( kit, XLink::OffEndXLink );
}


void DBWalk::InitWalk( const Actions *actions )
{
    WalkKit kit { actions, REQUIRE_SOLO };

    VisitBase( kit, XLink::MMAX_Link );
    VisitBase( kit, XLink::OffEndXLink );
}


void DBWalk::ZoneWalk( const Actions *actions,
                       const TreeZone &zone )
{
    WalkKit kit { actions, REQUIRE_SOLO };

	VisitBase( kit, zone.GetBase() );
}


void DBWalk::ExtraZoneWalk( const Actions *actions,
                             XLink extra_base_xlink )
{
    WalkKit kit { actions, STOP_IF_ALREADY_IN };

	VisitBase( kit, extra_base_xlink ); 
}


void DBWalk::SingleXLinkWalk( const Actions *actions, 
                              XLink xlink )
{
    WalkKit kit { actions, NO_RECURSE };

	VisitBase( kit, xlink );
}                      


void DBWalk::VisitBase( const WalkKit &kit, 
                        XLink root_xlink )
{
    VisitLink( kit, 
             { ROOT, 
               root_xlink, 
               XLink(), 
               nullptr,
               ContainerInterface::iterator(), 
               ContainerInterface::iterator(), 
               nullptr } );
}


void DBWalk::VisitSingular( const WalkKit &kit, 
                              const TreePtrInterface *p_x_singular, 
                              XLink xlink )
{
    ASSERT( p_x_singular );
    
    // MakeValueArchetype() can generate nodes with NULL pointers (eg in PointerIs)
    // and these get into the domain even though they are not allowed in input trees.
    // In this case, stop recursing since there's no child to build a row for.    
    if( !*p_x_singular )
        return;
        		
    TreePtr<Node> x = xlink.GetChildX();
    XLink child_xlink( x, p_x_singular );   
    
    VisitLink( kit, 
             { SINGULAR, 
               child_xlink, 
               xlink, 
               nullptr,
               ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular } );
}


void DBWalk::VisitSequence( const WalkKit &kit, 
                          SequenceInterface *x_seq, 
                          XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    SequenceInterface::iterator xit_predecessor = x_seq->end();
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        VisitLink( kit, 
                 { IN_SEQUENCE, 
                   child_xlink, 
                   xlink, 
                   x_seq,
                   xit_predecessor,
                   xit,
                   &*xit } );
        xit_predecessor = xit;
    }
}


void DBWalk::VisitCollection( const WalkKit &kit, 
                            CollectionInterface *x_col, 
                            XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        VisitLink( kit, 
                 { IN_COLLECTION, 
                   child_xlink, 
                   xlink, 
                   x_col,
				   ContainerInterface::iterator(),
                   xit,
                   &*xit } );
    }
}


void DBWalk::VisitLink( const WalkKit &kit, 
                        const WalkInfo &walk_info )
{
    INDENT(".");
    
    // This will also prevent recursion into xlink
    if( kit.mode==STOP_IF_ALREADY_IN && 
        kit.actions->is_unreached && 
        !kit.actions->is_unreached(walk_info) )
        return; // Terminate into existing links/nodes
            
    //TRACE("Visiting link ")(walk_info.xlink)("\n");    
            
    WindInActions( kit, walk_info );        
            
    // Recurse into our child nodes
    if( kit.mode != NO_RECURSE )
        VisitItemise( kit, walk_info.xlink ); 

    UnwindActions( kit, walk_info );                          
}


void DBWalk::VisitItemise( const WalkKit &kit, 
                           XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            VisitSequence( kit, x_seq, xlink );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            VisitCollection( kit, x_col, xlink );
        else if( TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            VisitSingular( kit, p_x_singular, xlink );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


void DBWalk::WindInActions( const WalkKit &kit, 
                            const WalkInfo &walk_info )
{
    // Wind-in actions
    if( kit.actions->domain_in )
		kit.actions->domain_in( walk_info );        
            
    DBCommon::DepthFirstOrderedIt df_it;
    
    if( kit.actions->indexes_in )
		df_it = kit.actions->indexes_in( walk_info );        
            
    if( kit.actions->link_row_in )
		kit.actions->link_row_in( walk_info, df_it );        
            
    if( kit.actions->node_row_in )
		kit.actions->node_row_in( walk_info );        

    if( kit.actions->indexes_in_late )
		kit.actions->indexes_in_late( walk_info );        
}                            


void DBWalk::UnwindActions( const WalkKit &kit, 
                            const WalkInfo &walk_info )
{
    // Unwind actions
    if( kit.actions->link_row_out )
		kit.actions->link_row_out( walk_info );                    
}                            
