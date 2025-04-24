#include "inversion_pass.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

#define INVERT_ON_WIND_IN

using namespace SR;

InversionPass::InversionPass( XTreeDatabase *db_ ) :
    db( db_ )
{
}


void InversionPass::Run(shared_ptr<Mutator> origin_mutator, shared_ptr<Patch> *source_layout_ptr)
{
    LocatedPatch base_lze( origin_mutator, source_layout_ptr );
    WalkLocatedPatches( base_lze );
}


void InversionPass::WalkLocatedPatches( LocatedPatch lze )
{
    // Really just a search for FreeZonePatch that fills in the target base mutator from the 
    // enclosing thing (if it's root or a tree zone). 
    // Inversion strategy: this mutator is available for every free zone because we did free zone
    // merging (if parent was a free zone, we'd have no such mutator)
    if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(*lze.second) )
    {
#ifdef INVERT_ON_WIND_IN
        Invert(lze); 
#endif
        // Free zone: recurse 
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)    
        {
            // We don't know the base if we're coming from a free zone
            ASSERT( dynamic_pointer_cast<TreeZonePatch>(child_patch) )
                  ("FZ under another FZ (probably), cannot determine target");
            LocatedPatch child_lze( nullptr, &child_patch );
            WalkLocatedPatches( child_lze );
        } );
    
#ifndef INVERT_ON_WIND_IN
        // Invert the free zone while unwinding. We must make these changes in the 
        // unwind due #784
        Invert(lze); 
#endif
    }
    else if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*lze.second) )
    {
		auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
		ASSERT( mutable_tree_zone );
        // Recurse, co-looping over the children/terminii so we can fill in target bases
        vector<shared_ptr<Mutator>> terminii = mutable_tree_zone->GetTerminusMutators();
        FreeZonePatch::ChildPatchIterator it_child = tree_patch->GetChildrenBegin();        
        for( shared_ptr<Mutator> terminus : terminii )
        {
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
            
            LocatedPatch child_lze( terminus, &*it_child );
            WalkLocatedPatches( child_lze );
                        
            ++it_child;
        }
        ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
    }
    else
        ASSERTFAIL();
}


void InversionPass::Invert( LocatedPatch lze )
{
    // Checks
    ASSERT( lze.first && lze.second && *lze.second);
    shared_ptr<Mutator> base_mutator = lze.first;
    ASSERT( base_mutator )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
    auto free_patch = dynamic_pointer_cast<FreeZonePatch>( *lze.second );
    ASSERT( free_patch )("Got LZE not a free zone: ")(lze); // WalkLocatedPatches() gave us wrong kind of patch
            
    // Collect base xlinks for child zones (which must be tree zones)
    vector<shared_ptr<Mutator>> terminii_mutators;
    vector<MutableTreeZone *> fixups;
    Patch::ForChildren(free_patch, [&](shared_ptr<Patch> &child_patch)    
    {
        // Inversion strategy: we're based on a free zone and FZ merging should 
        // have ensured we'll see only tree zones as children. Each base is a terminus 
        // for the new tree zone.
		auto child_tree_patch = dynamic_pointer_cast<TreeZonePatch>(child_patch);
        ASSERT( child_tree_patch ); 
        auto child_mutable_tree_zone = dynamic_cast<MutableTreeZone *>(child_tree_patch->GetZone());
        ASSERT(child_mutable_tree_zone);        
        
        terminii_mutators.push_back( child_mutable_tree_zone->GetBaseMutator() );
        fixups.push_back( child_mutable_tree_zone );
    } );
         
    // Make the inverted TZ    
    MutableTreeZone target_tree_zone( base_mutator, move(terminii_mutators) );    
//    ASSERT( !target_tree_zone.IsEmpty() )(target_tree_zone); // hypothesis: I don't think we can handle this #784
    FreeZone free_zone = *free_patch->GetZone();
    
    // Write it into the tree
    db->MainTreeExchange( &target_tree_zone, &free_zone, fixups );          
    

}
