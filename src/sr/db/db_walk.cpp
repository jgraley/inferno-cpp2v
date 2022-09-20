#include "db_walk.hpp"

using namespace SR;    

void DBWalk::FullWalk( const Actions &actions,
                       XLink root_xlink )
{
	AddAtRoot( actions, REQUIRE_SOLO, root_xlink );
    
    AddAtRoot( actions, REQUIRE_SOLO, XLink::MMAX_Link );
    AddAtRoot( actions, REQUIRE_SOLO, XLink::OffEndXLink );
}


void DBWalk::ExtraXLinkWalk( const Actions &actions,
                             XLink extra_xlink )
{
	// set to REQUIRE_SOLO to replicate #218
	AddAtRoot( actions, STOP_IF_ALREADY_IN, extra_xlink ); 
}


void DBWalk::AddAtRoot( const Actions &actions,
                        SubtreeMode mode, 
                        XLink root_xlink )
{
    AddLink( actions,
             mode, 
             { ROOT, 
               root_xlink, 
               XLink(), 
               nullptr,
               ContainerInterface::iterator(), 
               ContainerInterface::iterator(), 
               nullptr } );
}


void DBWalk::AddSingularNode( const Actions &actions,
                              SubtreeMode mode, 
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
    
    AddLink( actions,
             mode, 
             { SINGULAR, 
               child_xlink, 
               xlink, 
               nullptr,
               ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular } );
}


void DBWalk::AddSequence( const Actions &actions,
                          SubtreeMode mode, 
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
        AddLink( actions,
                 mode, 
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


void DBWalk::AddCollection( const Actions &actions,
                            SubtreeMode mode, 
                            CollectionInterface *x_col, 
                            XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        AddLink( actions,
                 mode, 
                 { IN_COLLECTION, 
                   child_xlink, 
                   xlink, 
                   x_col,
				   ContainerInterface::iterator(),
                   xit,
                   &*xit } );
    }
}


void DBWalk::AddLink( const Actions &actions,
                      SubtreeMode mode, 
                      const WalkInfo &walk_info )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && actions.domain_in_is_ok && !actions.domain_in_is_ok(walk_info) )
        return; // Terminate into the existing domain
            
    // Wind-in actions
    if( actions.domain_in )
		actions.domain_in( walk_info );        
            
    list<XLink>::const_iterator df_it;
    
    if( actions.indexes_in )
		df_it = actions.indexes_in( walk_info );        
            
    if( actions.xlink_row_in )
		actions.xlink_row_in( walk_info, df_it );        
            
    if( actions.node_row_in )
		actions.node_row_in( walk_info );        
            
    // Recurse into our child nodes
    AddChildren( actions, mode, walk_info.xlink ); 

    // Unwind actions
    if( actions.xlink_row_out )
		actions.xlink_row_out( walk_info );                   
}


void DBWalk::AddChildren( const Actions &actions,
                          SubtreeMode mode, 
                          XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            AddSequence( actions, mode, x_seq, xlink );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            AddCollection( actions, mode, x_col, xlink );
        else if( TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            AddSingularNode( actions, mode, p_x_singular, xlink );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


