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
                                           XLink source_base_xlink )
{
    TerminiiMap empty_terminii_map;
	TreePtr<Node> source_base = source_base_xlink.GetChildX();
    return DuplicateSubtreeWorker( dirty_grass, source_base, empty_terminii_map );
}
    
    
TreePtr<Node> Duplicate::DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           TreePtr<Node> source_base )
{
    TerminiiMap empty_terminii_map;
    return DuplicateSubtreeWorker( dirty_grass, source_base, empty_terminii_map );
}
    
    
TreePtr<Node> Duplicate::DuplicateSubtree( const DirtyGrassUpdateInterface *dirty_grass,
                                           XLink source_base_xlink,
                                           TerminiiMap &terminii_map )
{
    ASSERTS( source_base_xlink );

    // Terminus at root: we can subsitute but we can't make an updater.
    if( terminii_map.count(source_base_xlink) == 1 ) 
    {
        TerminusInfo &terminus_info = terminii_map.at(source_base_xlink);
        TreePtr<Node> dest_terminus = terminus_info.dest;
        TRACES("Reached source terminus ")(source_base_xlink)
              (" and substituting ")(dest_terminus)("\n");
        ASSERT( terminus_info.dest ); // Can't make an Terminus for the base
        terminus_info.dest = TreePtr<Node>();
        return dest_terminus;
    }

	TreePtr<Node> source_base = source_base_xlink.GetChildX();

    return DuplicateSubtreeWorker( dirty_grass,
                                   source_base,
                                   terminii_map );
}


TreePtr<Node> Duplicate::DuplicateSubtreeWorker( const DirtyGrassUpdateInterface *dirty_grass,
                                                 TreePtr<Node> source,
                                                 TerminiiMap &terminii_map )
{
    // Make a new node, since we're substituting, preserve dirtyness        
    TreePtr<Node> dest = DuplicateNode( dirty_grass, source, false );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_items = source->Itemise();
    vector< Itemiser::Element * > dest_items = dest->Itemise(); 

    TRACES("Duplicating %d members source=", dest_items.size())(source)(" dest=")(*dest)("\n");
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
                XLink source_child_xlink = XLink(source, &source_elt);

                if( terminii_map.count(source_child_xlink) == 1 ) 
                {
                    TerminusInfo &terminus_info = terminii_map.at(source_child_xlink);
                    TreePtr<Node> dest_terminus = terminus_info.dest;
                    TRACES("Reached source terminus ")(source_child_xlink)
                          (" and substituting ")(dest_terminus)("\n");
                    terminus_info.dest = TreePtr<Node>();
                    ContainerInterface::iterator dest_it = dest_container->insert( dest_terminus );
                    // NULL value was provided, so consider it a placeholder for an updater
                    if( !dest_terminus )
                        terminus_info.updater = make_shared<ContainerTerminus>( dest_container, dest_it );
                }
                else
                {
                     //TRACES("Duplicating ")(*source_elt)("\n");
                    TreePtr<Node> dest_elt = DuplicateSubtreeWorker( dirty_grass,
                                                                     source_child_xlink.GetChildX(), 
                                                                     terminii_map );
                    //TRACE("inserting ")(*dest_elt)(" directly\n");
                    dest_container->insert( dest_elt );
                }
            }
        }            
        else if( TreePtrInterface *source_singular = dynamic_cast<TreePtrInterface *>(source_items[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_singular)("\n");
            TreePtrInterface *dest_singular = dynamic_cast<TreePtrInterface *>(dest_items[i]);
            ASSERTS( *source_singular )("source should be non-nullptr");
            XLink source_child_xlink = XLink(source, source_singular);
            
            if( terminii_map.count(source_child_xlink) == 1 ) 
            {
                TerminusInfo &terminus_info = terminii_map.at(source_child_xlink);
                TreePtr<Node> dest_terminus = terminus_info.dest;
                TRACES("Reached source terminus ")(source_child_xlink)
                      (" and substituting ")(dest_terminus)("\n");
                terminus_info.dest = TreePtr<Node>();
                *dest_singular = dest_terminus;
                // NULL value was provided, so consider it a placeholder for an updater
                if( !dest_terminus )
                    terminus_info.updater = make_shared<SingularTerminus>( dest_singular );
            }
            else
            {
                *dest_singular = DuplicateSubtreeWorker( dirty_grass,
                                                         source_child_xlink.GetChildX(), 
                                                         terminii_map );
				ASSERTS( *dest_singular );
				ASSERTS( TreePtr<Node>(*dest_singular)->IsFinal() );            
            }
        }
        else
        {
            ASSERTFAILS("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
}