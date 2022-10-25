#include "db_walk.hpp"

using namespace SR;    

void DBWalk::FullWalk( const Actions *actions,
                       XLink root_xlink )
{
    InitWalk( actions );
    
    WalkKit kit { actions, REQUIRE_SOLO };
    
	VisitBase( kit, root_xlink, ROOT );
}


void DBWalk::InitWalk( const Actions *actions )
{
    WalkKit kit { actions, REQUIRE_SOLO };

    VisitBase( kit, XLink::MMAX_Link, ROOT );
    VisitBase( kit, XLink::OffEndXLink, ROOT );
}


void DBWalk::ZoneWalk( const Actions *actions,
                       const TreeZone &zone )
{
    WalkKit kit { actions, REQUIRE_SOLO };

	VisitBase( kit, zone.GetBase(), UNKNOWN );
}


void DBWalk::ExtraFullWalk( const Actions *actions,
                            XLink extra_base_xlink,
                            const unordered_set<XLink> *exclusions )
{
    WalkKit kit { actions, STOP_IF_ALREADY_IN, exclusions };
	TRACE("base=")(extra_base_xlink)("\n");
	VisitBase( kit, extra_base_xlink, ROOT );  
    ASSERTFAIL();
}


void DBWalk::ExtraZoneWalk( const Actions *actions,
                            const TreeZone &extra_zone )
{
    // TODO go over to natively using zones for walk. 
    unordered_set<XLink> terminii_unordered;
    for( XLink terminus : extra_zone.GetTerminii() )
        terminii_unordered.insert( terminus );
        
    WalkKit kit { actions, STOP_IF_ALREADY_IN, &terminii_unordered };
	TRACE("base=")(extra_zone.GetBase())(" terminii=")(terminii_unordered)("\n");
	VisitBase( kit, extra_zone.GetBase(), ROOT );  
}


void DBWalk::VisitBase( const WalkKit &kit, 
                        XLink root_xlink,
                        ContainmentContext context )
{
    VisitLink( kit, 
             { XLink(), 
               -1,
               context,	 
               nullptr,
               -1,
               ContainerInterface::iterator(), 
               ContainerInterface::iterator(),
               root_xlink.GetXPtr(), 
               root_xlink, 
               root_xlink.GetChildX() } );
}


void DBWalk::VisitSingular( const WalkKit &kit, 
                            const TreePtrInterface *p_x_singular, 
                            XLink xlink,
                            int item_ordinal )
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
             { xlink, 
               item_ordinal,
               SINGULAR,
               nullptr,
               -1,
               ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular, 
               child_xlink, 
               child_xlink.GetChildX() } );
}


void DBWalk::VisitSequence( const WalkKit &kit, 
                            SequenceInterface *x_seq, 
                            XLink xlink,
                            int item_ordinal )
{
    TreePtr<Node> x = xlink.GetChildX();
    SequenceInterface::iterator xit_predecessor = x_seq->end();    
    int i=0;
    for( SequenceInterface::iterator xit = x_seq->begin();
         xit != x_seq->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        VisitLink( kit, 
                 { xlink, 
                   item_ordinal,
                   IN_SEQUENCE,
                   x_seq,
                   i,
                   xit_predecessor,
                   xit,
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildX() } );
        xit_predecessor = xit;
        i++;
    }
}


void DBWalk::VisitCollection( const WalkKit &kit, 
                              CollectionInterface *x_col, 
                              XLink xlink,
                              int item_ordinal )
{
    TreePtr<Node> x = xlink.GetChildX();    
    int i=0;
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        VisitLink( kit, 
                 { xlink, 
                   item_ordinal,
                   IN_COLLECTION,
                   x_col,
                   i,
				   ContainerInterface::iterator(),
                   xit,
                   &*xit, 
                   child_xlink, 
                   child_xlink.GetChildX() } );
        i++;
    }
}


void DBWalk::VisitLink( const WalkKit &kit, 
                        const WalkInfo &walk_info )
{
    INDENT(".");
    
    // This will also prevent recursion into xlink
    if( kit.mode==STOP_IF_ALREADY_IN && 
        kit.exclusions->count( walk_info.xlink ) )
        return; // Terminate into existing links/nodes
            
    TRACE("Visiting link ")(walk_info.xlink)("\n");    
            
    WindInActions( kit, walk_info );        
            
    // Recurse into our child nodes
    VisitItemise( kit, walk_info.xlink, walk_info.x ); 

    UnwindActions( kit, walk_info );                          
}


void DBWalk::VisitItemise( const WalkKit &kit, 
                           XLink xlink, 
                           TreePtr<Node> x )
{
    ASSERT(x)("This probably means we're walking an incomplete tree");
    vector< Itemiser::Element * > x_items = x->Itemise();
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


void DBWalk::WindInActions( const WalkKit &kit, 
                            const WalkInfo &walk_info )
{
    // Wind-in actions
    if( kit.actions->domain_in )
		kit.actions->domain_in( walk_info );        
               
    if( kit.actions->link_row_in )
		kit.actions->link_row_in( walk_info );        
            
    if( kit.actions->node_row_in )
		kit.actions->node_row_in( walk_info );        

    if( kit.actions->indexes_in )
		kit.actions->indexes_in( walk_info );                    
}                            


void DBWalk::UnwindActions( const WalkKit &kit, 
                            const WalkInfo &walk_info )
{                  
    if( kit.actions->indexes_out )
		kit.actions->indexes_out( walk_info );                    

    if( kit.actions->node_row_out )
		kit.actions->node_row_out( walk_info );        

    if( kit.actions->link_row_out )
		kit.actions->link_row_out( walk_info );        

    if( kit.actions->domain_out )
		kit.actions->domain_out( walk_info );        
}                            
