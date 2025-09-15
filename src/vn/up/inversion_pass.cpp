#include "inversion_pass.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "scaffold_ops.hpp"
#include "up_common.hpp"

#include <iostream>

using namespace VN;

InversionPass::InversionPass( XTreeDatabase *db_, ScaffoldOps *sops_ ) :
    db( db_ ),
    sops( sops_ )
{
}


void InversionPass::RunInversion(XLink origin, shared_ptr<Patch> *source_layout_ptr)
{
	INDENT("I");
    LocatedPatch base_lze( origin, source_layout_ptr );
    WalkLocatedPatches( base_lze );
}


void InversionPass::WalkLocatedPatches( LocatedPatch lze )
{
    // Really just a search for FreePatch that fills in the target base xlink from the 
    // enclosing thing (if it's root or a tree zone). 
    // Inversion strategy: this xlink is available for every free zone because we did free zone
    // merging (if parent was a free zone, we'd have no such xlink)
    if( auto free_patch = dynamic_pointer_cast<FreePatch>(*lze.second) )
    {
        Invert(lze); 
        // Free zone: recurse 
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)    
        {
            // We don't know the base if we're coming from a free zone
            ASSERT( dynamic_pointer_cast<TreePatch>(child_patch) )
                  ("FZ under another FZ (probably), cannot determine target");
            LocatedPatch child_lze( XLink(), &child_patch );
            WalkLocatedPatches( child_lze );
        } );
    }
    else if( auto tree_patch = dynamic_pointer_cast<TreePatch>(*lze.second) )
    {
		auto tree_zone = tree_patch->GetZone();
		ASSERT( tree_zone );
        // Recurse, co-looping over the children/terminii so we can fill in target bases
        FreePatch::ChildPatchIterator it_child = tree_patch->GetChildrenBegin();        
        for( size_t i=0; i<tree_zone->GetNumTerminii(); i++ )
        {        
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
            
            LocatedPatch child_lze( tree_zone->GetTerminusXLink(i), &*it_child );
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
    auto free_patch = dynamic_pointer_cast<FreePatch>( *lze.second );
	DBCommon::TreeOrdinal ordinal_to_tear_down;
    ASSERT( free_patch )("Got LZE not a free zone: ")(lze); // WalkLocatedPatches() gave us wrong kind of patch
	{
		XLink base_xlink = lze.first;
		ASSERT( base_xlink )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
		FreeZone new_free_zone = *free_patch->GetZone();
				
		// Collect base xlinks for child zones (which must be tree zones)
		vector<XLink> terminii;
		vector<TreeZone *> fixups;
		Patch::ForChildren(free_patch, [&](shared_ptr<Patch> &child_patch)    
		{
			// Inversion strategy: we're based on a free zone and FZ merging should 
			// have ensured we'll see only tree zones as children. Each base is a terminus 
			// for the new tree zone.
			auto child_tree_patch = dynamic_pointer_cast<TreePatch>(child_patch);
			ASSERT( child_tree_patch ); 
			
			terminii.push_back( child_tree_patch->GetZone()->GetBaseXLink() );
			fixups.push_back( child_tree_patch->GetZone() );
		} );             
			 
		// Make the inverted TZ - this contains the old content identified by inversion
		TreeZone main_tree_zone = TreeZone( base_xlink, 
											move(terminii), 
											db->GetMainTreeOrdinal() );
		
		// Write free zone with new content into an extra tree. main_tree_zone is 
		// used here only for the plug types.
		TreeZone tree_zone_in_extra = sops->FreeZoneIntoExtraTree( new_free_zone, main_tree_zone );
		ordinal_to_tear_down = tree_zone_in_extra.GetTreeOrdinal();
		
		// Swap in the true moving zone. After this swap:
		// tree_zone_in_extra <- the old content
		// main_tree_zone <- the new content
		db->XTreeDatabase::SwapTreeToTree( main_tree_zone, fixups,
										   tree_zone_in_extra, vector<TreeZone *>() );
	
		// Update the patch to be a tree zone patch with the new content in situ.
		*(lze.second) = free_patch->ConvertToTree( main_tree_zone );
		
		// XLink memory safety: let zone and root_xlink drop out of scope 
		// before freeing tree, which will delete the underlying TreePtr<>	
	}

	// Delete old content from DB, but DO defer freeing of nodes (XLink memory safety)
	db->TeardownTree( ordinal_to_tear_down, true );   

}
