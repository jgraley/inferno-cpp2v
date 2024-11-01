#include "db_walk.hpp"

using namespace SR;    

void DBWalk::Walk( const Actions *actions,
                   XLink base_xlink,
                   Context base_context,
                   const RootRecord *root_record )
{
    WalkKit kit { actions, base_xlink, root_record };
	VisitBase( kit, base_context );  
}


void DBWalk::VisitBase( const WalkKit &kit,                         
                        Context context )
{
    XLink base_xlink = kit.base_xlink;
    VisitLink( kit, 
             { TreePtr<Node>(), 
               -1,
               context,	 
               nullptr,
               -1,
               ContainerInterface::iterator(), 
               ContainerInterface::iterator(),
               base_xlink.GetTreePtrInterface(), 
               base_xlink, 
               base_xlink.GetChildTreePtr() } );
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
               child_xlink.GetChildTreePtr() } ); 
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
                   child_xlink.GetChildTreePtr() } );
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
                   child_xlink.GetChildTreePtr() } ); // should be child_xlink's child
        i++;
    }
}


void DBWalk::VisitLink( const WalkKit &kit, 
                        const WalkInfo &walk_info ) // .x should be .xlink's child
{
    INDENT(".");            
    TRACE("Visiting link ")(walk_info.xlink)("\n");    
            
    WindInActions( kit, walk_info );        
            
    // Recurse into our child nodes
    VisitItemise( kit, walk_info.xlink ); 

    UnwindActions( kit, walk_info );                          
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

    if( kit.actions->domain_extension_in )
		kit.actions->domain_extension_in( walk_info );        
}                            


void DBWalk::UnwindActions( const WalkKit &kit, 
                            const WalkInfo &walk_info )
{                  
    if( kit.actions->domain_extension_out )
		kit.actions->domain_extension_out( walk_info );        

    if( kit.actions->indexes_out )
		kit.actions->indexes_out( walk_info );                    

    if( kit.actions->node_row_out )
		kit.actions->node_row_out( walk_info );        

    if( kit.actions->link_row_out )
		kit.actions->link_row_out( walk_info );        

    if( kit.actions->domain_out )
		kit.actions->domain_out( walk_info );        
}                            
