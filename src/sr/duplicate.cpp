#include "duplicate.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> Duplicate::DuplicateNode( const SCREngine *scr_engine,
                                        TreePtr<Node> source,
                                        bool force_dirty )
{
    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
    ASSERTS(dest);

    bool source_dirty = scr_engine->IsDirtyGrass( source );
    if( force_dirty || // requested by caller
        source_dirty ) // source was dirty
    {
        //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
        scr_engine->AddDirtyGrass( dest );
    }
    
    return dest;    
}                                                     

                                             
TreePtr<Node> Duplicate::DuplicateSubtree( const SCREngine *scr_engine,
                                           XLink source_xlink,
                                           XLink source_terminus_xlink,
                                           TreePtr<Node> dest_terminus,
                                           int *terminus_hit_count )
{
    ASSERTS( source_xlink );
	TreePtr<Node> source = source_xlink.GetChildX();
        
    TreePtr<Node> source_terminus;
    if( source_terminus_xlink )
    {
		source_terminus = source_terminus_xlink.GetChildX();
        ASSERTS( dest_terminus );
	}
    
    // If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copies of source terminus (directly, without duplicating).
    //if( source_terminus )
		//FTRACE(source);
    if( source_terminus_xlink && source_xlink == source_terminus_xlink ) 
    {
        TRACES("Reached source terminus ")(source_terminus_xlink)
              (" and substituting ")(dest_terminus)("\n");
        if( terminus_hit_count )
			(*terminus_hit_count)++;
        return dest_terminus;
    }

    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( scr_engine, source, false );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_items = source->Itemise();
    vector< Itemiser::Element * > dest_items = dest->Itemise(); 

    TRACES("Duplicating %d members source=", dest_items.size())(source_xlink)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-nullptr members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_items.size(); i++ )
    {
        //TRACES("Duplicating member %d\n", i );
        ASSERTS( source_items[i] )( "itemise returned null element" );
        ASSERTS( dest_items[i] )( "itemise returned null element" );
        
        if( ContainerInterface *source_container = dynamic_cast<ContainerInterface *>(source_items[i]) )                
        {
            ContainerInterface *dest_container = dynamic_cast<ContainerInterface *>(dest_items[i]);

            dest_container->clear();

            //TRACE("Duplicating container size %d\n", keynode_con->size() );
            for( const TreePtrInterface &source_elt : *source_container )
            {
                ASSERTS( source_elt ); // present simplified scheme disallows nullptr
                //TRACES("Duplicating ")(*source_elt)("\n");
                TreePtr<Node> dest_elt = DuplicateSubtree( scr_engine,
                                                           XLink( source, &source_elt ), 
                                                           source_terminus_xlink, 
                                                           dest_terminus,
                                                           terminus_hit_count );
                //TRACE("inserting ")(*dest_elt)(" directly\n");
                dest_container->insert( dest_elt );
            }
        }            
        else if( TreePtrInterface *source_singular = dynamic_cast<TreePtrInterface *>(source_items[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
            ASSERTS( *source_singular )("source should be non-nullptr");
            *dest_singular = DuplicateSubtree( scr_engine,
                                               XLink(source, source_singular), 
                                               source_terminus_xlink, 
                                               dest_terminus,
                                               terminus_hit_count );
            ASSERTS( *dest_singular );
            ASSERTS( TreePtr<Node>(*dest_singular)->IsFinal() );            
        }
        else
        {
            ASSERTFAILS("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
}
