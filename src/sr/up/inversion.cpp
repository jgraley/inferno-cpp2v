#include "inversion.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneInverter::TreeZoneInverter( XTreeDatabase *db_ ) :
    db( db_ )
{
}


void TreeZoneInverter::Run(XLink target_origin, shared_ptr<Patch> *source_layout_ptr)
{
    LocatedPatch base_lze( target_origin, source_layout_ptr );
    WalkLocatedPatches( base_lze );
    
    // For each targetted patch in the layout, perform replace operation on the DB
    Patch::ForDepthFirstWalk( *source_layout_ptr, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto replace_patch = dynamic_pointer_cast<TargettedPatch>(patch) )
        {
            db->MainTreeExchange( replace_patch->GetTargetTreeZone(), replace_patch->GetSourceZone() );
        }
    } );    
}


void TreeZoneInverter::WalkLocatedPatches( LocatedPatch lze )
{
    // Really just a search for FreeZonePatch that fills in the target base XLink from the 
    // enclosing thing (if it's root or a tree zone). 
    // Inversion strategy: this XLink is available for every free zone because we did free zone
    // merging (if parent was a free zone, we'd have no XLink)
    if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(*lze.second) )
    {
        // Free zone: recurse and then invert locally
        free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)    
        {
            // We don't know the base if we're coming from a free zone
            ASSERT( dynamic_pointer_cast<TreeZonePatch>(child_patch) )
                  ("FZ under another FZ (probably), cannot determine target XLink");
            LocatedPatch child_lze( XLink(), &child_patch );
            WalkLocatedPatches( child_lze );
        } );
    
        // Invert the free zone while unwinding
        Invert(lze); 
    }
    else if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*lze.second) )
    {
        // Recurse, co-looping over the children/terminii so we can fill in target bases
        vector<XLink> terminii = tree_patch->GetZone()->GetTerminusXLinks();
        FreeZonePatch::ChildExpressionIterator it_child = tree_patch->GetChildrenBegin();        
        for( XLink terminus_xlink : terminii )
        {
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
            
            LocatedPatch child_lze( terminus_xlink, &*it_child );
            WalkLocatedPatches( child_lze );
                        
            ++it_child;
        }
        ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
    }
    else
        ASSERTFAIL();
}


void TreeZoneInverter::Invert( LocatedPatch lze )
{
    // Checks
    ASSERT( lze.first && lze.second && *lze.second);
    XLink base_xlink = lze.first;
    ASSERT( base_xlink )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
    auto free_patch = dynamic_pointer_cast<FreeZonePatch>( *lze.second );
    ASSERT( free_patch )("Got LZE not a free zone: ")(lze); // WalkLocatedPatches() gave us wrong kind of patch
            
    // Collect base xlinks for child zones (which must be tree zones)
    vector<XLink> terminii_xlinks;
    free_patch->ForChildren([&](shared_ptr<Patch> &child_patch)    
    {
        auto child_tree_patch = dynamic_pointer_cast<TreeZonePatch>( child_patch );        
        // Inversion strategy: we're based on a free zone and FZ merging should 
        // have ensured we'll see only tree zones as children. Each base is a terminus 
        // for the new tree zone.
        ASSERT( child_tree_patch ); 
        
        terminii_xlinks.push_back( child_tree_patch->GetZone()->GetBaseXLink() );
    } );
         
    // Make the inverted TZ    
    unique_ptr<TreeZone> inverted_tree_zone = db->MakeMutableTreeZone( base_xlink, terminii_xlinks );    
    
    // Modify the expression to include inverted TZ as target
    *lze.second = make_shared<TargettedPatch>( move(inverted_tree_zone),
                                               make_unique<FreeZone>( *free_patch->GetZone() ),
                                               free_patch->MoveChildExpressions() );           
}
