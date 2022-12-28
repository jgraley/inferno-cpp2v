#include "duplicate.hpp"
#include "scr_engine.hpp"

using namespace SR;

TreePtr<Node> Duplicate::DuplicateNode( const DirtyGrassUpdateInterface *dirty_grass,
                                        TreePtr<Node> source,
                                        bool force_dirty )
{
    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
    ASSERTS(dest);

    if( dirty_grass )
    {
        bool source_dirty = dirty_grass->IsDirtyGrass( source );
        if( force_dirty || // requested by caller
            source_dirty ) // source was dirty
        {
            //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
            dirty_grass->AddDirtyGrass( dest );
        }
    }
    
    return dest;    
}                                                     

                                             
TreePtr<Node> Duplicate::DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink,
                                           XLink source_terminus_xlink,
                                           TreePtr<Node> dest_terminus,
                                           int *terminus_hit_count )
{
    if( source_terminus_xlink )
        ASSERTS( dest_terminus );
    map<XLink, TreePtr<Node>> terminii;
    if( source_terminus_xlink )
        terminii[source_terminus_xlink] = dest_terminus;
    TreePtr<Node> r = DuplicateSubtree( dirty_grass, source_xlink, terminii );
    if( terminus_hit_count )
        for( auto p : terminii )
            if( !p.second ) // these are switched to NULL on hitting terminus
                (*terminus_hit_count)++;
    return r;
}


TreePtr<Node> Duplicate::DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink )
{
    map<XLink, TreePtr<Node>> empty_terminii;
    return DuplicateSubtree( dirty_grass, source_xlink, empty_terminii );
}
    
    
TreePtr<Node> Duplicate::DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_xlink,
                                           map<XLink, TreePtr<Node>> &terminii )
{
    ASSERTS( source_xlink );
	TreePtr<Node> source = source_xlink.GetChildX();
            
    // If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copies of source terminus (directly, without duplicating).

    if( terminii.count(source_xlink) == 1 ) 
    {
        TreePtr<Node> dest_terminus = terminii.at(source_xlink);
        TRACES("Reached source terminus ")(source_xlink)
              (" and substituting ")(dest_terminus)("\n");
        ASSERT( dest_terminus ); // if this fails, we're probably hitting the same terminus twice
        terminii.at(source_xlink) = TreePtr<Node>();
        return dest_terminus;
    }

    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( dirty_grass, source, false );

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
                TreePtr<Node> dest_elt = DuplicateSubtree( dirty_grass,
                                                           XLink( source, &source_elt ), 
                                                           terminii );
                //TRACE("inserting ")(*dest_elt)(" directly\n");
                dest_container->insert( dest_elt );
            }
        }            
        else if( TreePtrInterface *source_singular = dynamic_cast<TreePtrInterface *>(source_items[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
            ASSERTS( *source_singular )("source should be non-nullptr");
            *dest_singular = DuplicateSubtree( dirty_grass,
                                               XLink(source, source_singular), 
                                               terminii );
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
