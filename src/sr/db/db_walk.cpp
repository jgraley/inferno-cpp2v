#include "db_walk.hpp"

using namespace SR;    

void DBWalk::AddAtRoot( SubtreeMode mode, 
                        XLink root_xlink )
{
    AddLink( mode, 
             { ROOT, 
               root_xlink, 
               XLink(), 
               nullptr,
               ContainerInterface::iterator(), 
               ContainerInterface::iterator(), 
               nullptr } );
}


void DBWalk::AddSingularNode( SubtreeMode mode, 
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
    
    AddLink( mode, 
             { SINGULAR, 
               child_xlink, 
               xlink, 
               nullptr,
               ContainerInterface::iterator(),
               ContainerInterface::iterator(),
               p_x_singular } );
}


void DBWalk::AddSequence( SubtreeMode mode, 
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
        AddLink( mode, 
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


void DBWalk::AddCollection( SubtreeMode mode, 
                            CollectionInterface *x_col, 
                            XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    for( CollectionInterface::iterator xit = x_col->begin();
         xit != x_col->end();
         ++xit )
    {
        XLink child_xlink( x, &*xit );
        AddLink( mode, 
                 { IN_COLLECTION, 
                   child_xlink, 
                   xlink, 
                   x_col,
				   ContainerInterface::iterator(),
                   xit,
                   &*xit } );
    }
}


void DBWalk::AddLink( SubtreeMode mode, 
                      const WalkInfo &walk_info )
{
    // This will also prevent recursion into xlink
    if( mode==STOP_IF_ALREADY_IN && false )
        return; // Terminate into the existing domain
            
    // Recurse into our child nodes
    AddChildren( mode, walk_info.xlink ); 
}


void DBWalk::AddChildren( SubtreeMode mode, 
                          XLink xlink )
{
    TreePtr<Node> x = xlink.GetChildX();
    vector< Itemiser::Element * > x_items = x->Itemise();
    for( Itemiser::Element *xe : x_items )
    {
        if( SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(xe) )
            AddSequence( mode, x_seq, xlink );
        else if( CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(xe) )
            AddCollection( mode, x_col, xlink );
        else if( TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(xe) )
            AddSingularNode( mode, p_x_singular, xlink );
        else
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a singular TreePtr");
    }
}


