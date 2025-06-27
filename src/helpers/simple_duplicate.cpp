
#include "simple_duplicate.hpp"
#include "node/specialise_oostd.hpp"
#include "common/trace.hpp"

TreePtr<Node> SimpleDuplicate::DuplicateNode( TreePtr<Node> source ) 
{
    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    auto dest = TreePtr<Node>( dynamic_pointer_cast<Node>( dup_dest ) );
    ASSERT(dest);
    
    return dest;    
}                                                     


TreePtr<Node> SimpleDuplicate::DuplicateSubtree( TreePtr<Node> source ) 
{    
	INDENT("D");
    ASSERT( source );
   
    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( source );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > keynode_items = source->Itemise();
    vector< Itemiser::Element * > dest_items = dest->Itemise(); 

    TRACE("Duplicating %d members source=", dest_items.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( vector< Itemiser::Element * >::size_type i=0; i<dest_items.size(); i++ )
    {
        //TRACE("Duplicating member %d\n", i );
        ASSERT( keynode_items[i] )( "itemise returned null element" );
        ASSERT( dest_items[i] )( "itemise returned null element" );
        
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_items[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_items[i]);

            dest_con->clear();

            //TRACE("Duplicating container size %d\n", keynode_con->size() );
            for( const TreePtrInterface &p : *keynode_con )
            {
                ASSERT( p ); // present simplified scheme disallows nullptr
                //TRACE("Duplicating ")(*p)("\n");
                TreePtr<Node> n = DuplicateSubtree( (TreePtr<Node>)p );
                //TRACE("Normal element, inserting ")(*n)(" directly\n");
                dest_con->insert( n );
            }
        }            
        else if( TreePtrInterface *keynode_singular = dynamic_cast<TreePtrInterface *>(keynode_items[i]) )
        {
            TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
            ASSERT( *keynode_singular )("source should be non-nullptr");
            *dest_singular = DuplicateSubtree( (TreePtr<Node>)*keynode_singular );
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
