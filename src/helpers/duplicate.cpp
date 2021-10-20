
#include "duplicate.hpp"
#include "node/specialise_oostd.hpp"
#include "common/trace.hpp"

TreePtr<Node> Duplicate::DuplicateNode( TreePtr<Node> source ) 
{
    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    auto dest = TreePtr<Node>( dynamic_pointer_cast<Node>( dup_dest ) );
    ASSERT(dest);
    
    return dest;    
}                                                     


TreePtr<Node> Duplicate::DuplicateSubtree( TreePtr<Node> source,
                                           TreePtr<Node> source_terminus,
                                           TreePtr<Node> dest_terminus ) 
{
    ASSERT( source );
    if( source_terminus )
        ASSERT( dest_terminus );
   
    // If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copies of source terminus (directly, without duplicating).
    if( source_terminus && source == source_terminus ) 
        return dest_terminus;

    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( source );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > keynode_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Duplicating %d members source=", dest_memb.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        //TRACE("Duplicating member %d\n", i );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);

            dest_con->clear();

            //TRACE("Duplicating container size %d\n", keynode_con->size() );
            FOREACH( const TreePtrInterface &p, *keynode_con )
            {
                ASSERT( p ); // present simplified scheme disallows nullptr
                //TRACE("Duplicating ")(*p)("\n");
                TreePtr<Node> n = DuplicateSubtree( (TreePtr<Node>)p, source_terminus, dest_terminus );
                //TRACE("Normal element, inserting ")(*n)(" directly\n");
                dest_con->insert( n );
            }
        }            
        else if( TreePtrInterface *keynode_singular = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_singular )("source should be non-nullptr");
            *dest_singular = DuplicateSubtree( (TreePtr<Node>)*keynode_singular, source_terminus, dest_terminus );
            ASSERT( *dest_singular );
            ASSERT( TreePtr<Node>(*dest_singular)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
}
