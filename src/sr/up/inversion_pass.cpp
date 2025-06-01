#include "inversion_pass.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "scaffold_ops.hpp"

#include <iostream>

using namespace SR;

InversionPass::InversionPass( XTreeDatabase *db_, ScaffoldOps *sops_ ) :
    db( db_ ),
    sops( sops_ )
{
}


void InversionPass::RunInversion(const Mutator &origin_mutator, shared_ptr<Patch> *source_layout_ptr)
{
	INDENT("I");
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
        Invert(lze); 
        // Free zone: recurse 
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)    
        {
            // We don't know the base if we're coming from a free zone
            ASSERT( dynamic_pointer_cast<TreeZonePatch>(child_patch) )
                  ("FZ under another FZ (probably), cannot determine target");
            LocatedPatch child_lze( Mutator(), &child_patch );
            WalkLocatedPatches( child_lze );
        } );
    }
    else if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*lze.second) )
    {
		auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
		ASSERT( mutable_tree_zone );
        // Recurse, co-looping over the children/terminii so we can fill in target bases
        FreeZonePatch::ChildPatchIterator it_child = tree_patch->GetChildrenBegin();        
        for( size_t i=0; i<mutable_tree_zone->GetNumTerminii(); i++ )
        {        
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
            
            LocatedPatch child_lze( mutable_tree_zone->GetTerminusMutator(i), &*it_child );
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
    Mutator base_mutator = lze.first;
    ASSERT( base_mutator )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
    auto free_patch = dynamic_pointer_cast<FreeZonePatch>( *lze.second );
    ASSERT( free_patch )("Got LZE not a free zone: ")(lze); // WalkLocatedPatches() gave us wrong kind of patch
    FreeZone new_free_zone = *free_patch->GetZone();
            
    // Collect base xlinks for child zones (which must be tree zones)
    vector<Mutator> terminii_mutators;
    vector<TreeZone *> fixups;
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
        fixups.push_back( child_tree_patch->GetZone() );
    } );             
         
    // Make the inverted TZ    
    MutableTreeZone main_tree_zone_m( move(base_mutator), move(terminii_mutators), DBCommon::TreeOrdinal::MAIN );    
	XTreeZone main_tree_zone = main_tree_zone_m.GetXTreeZone();
	
    // Write it into the tree
	XTreeZone tree_zone_in_extra = sops->FreeZoneIntoExtraTree( new_free_zone, main_tree_zone_m ).GetXTreeZone();
	
	// Swap in the true moving zone. Names become misleading because contents swap:
	// tree_zone_in_extra <- the actual moving zone now in extra tree
	// main_tree_zone_from <- the "from" scaffold now in main tree, to be killed by inversion
	db->XTreeDatabase::SwapTreeToTree( main_tree_zone, fixups,
		    						   tree_zone_in_extra, vector<TreeZone *>() );

	db->TeardownTree( tree_zone_in_extra.GetTreeOrdinal() );   
}
