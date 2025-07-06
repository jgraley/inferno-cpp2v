#include "move_out_pass.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "misc_passes.hpp"
#include "scaffold_ops.hpp"
#include "up_common.hpp"

#include <iostream>

using namespace SR;


MoveOutPass::MoveOutPass(XTreeDatabase *db_, class ScaffoldOps *sops_) :
	db( db_ ),
	sops( sops_ )
{
}


void MoveOutPass::Run(shared_ptr<Patch> &layout, MovesMap &moves_map)
{
	INDENT("M");

	vector<shared_ptr<Patch> *> out_of_order_patches;  
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreePatch>(patch);
        if( tree_patch->GetIntent() == TreePatch::Intent::MOVEABLE )
		{
			XLink base_xlink = tree_patch->GetZone()->GetBaseXLink();
			ASSERT( base_xlink );
		
			TRACE("Moving ")(patch)("\n");
			
			// We can move it to the new place, avoiding the need for duplication
			MoveTreeZoneOut(&patch, layout, moves_map);
		}
	} );
}


void MoveOutPass::MoveTreeZoneOut( shared_ptr<Patch> *ooo_patch_ptr, shared_ptr<Patch> &layout, MovesMap &moves_map)
{
	// Out-of-order patch is located at the "to" location, but contains the "from" tree zone.
	auto ooo_tree_patch = dynamic_pointer_cast<TreePatch>(*ooo_patch_ptr);
	ASSERT( ooo_tree_patch );
	TreeZone main_tree_zone_from = ooo_tree_patch->GetXTreeZone();
	ASSERT( !main_tree_zone_from.IsEmpty() ); // See #784
	
	// ------------------------- Create extra tree with plugged scaffold ---------------------------
	// Make scaffold free zones that fit in place of the moving zone
	auto scaffold_zone_from = sops->CreateSimilarScaffoldZone(main_tree_zone_from);
	TRACE("\"From\" scaffold: ")(scaffold_zone_from)("\n");
	TreeZone tree_zone_in_extra = sops->FreeZoneIntoExtraTree( scaffold_zone_from, main_tree_zone_from );
		
	// ------------------------- Swap "from" zone into our extra tree ---------------------------
	//FTRACE("main_tree_zone_from: ")(main_tree_zone_from)("\nfree_zone: ")(*free_zone)("\n");
	// Put the scaffold into the "from" part of the tree, displacing 
	// the original contents, which we shall move
	//main_tree_zone_from.Validate(db);
	
	// Determine the fix-ups we'll need to do for tree zones in neighbouring patches
    vector<TreeZone *> fixups;	
    for( size_t i=0; i<main_tree_zone_from.GetNumTerminii(); i++ )
	{				
		TreeZone *found = nullptr;
		TreePatch::ForTreeDepthFirstWalk(layout, [&](shared_ptr<TreePatch> &patch)
		{
			TreeZone *candidate = patch->GetZone();
			if( candidate->GetBaseXLink() == main_tree_zone_from.GetTerminusXLink(i) )
			{
				ASSERT( !found );
				found = candidate;
			}
		}, nullptr );
		
		fixups.push_back( found ); // does not have to be found; TZs can be disconnected
	}
			
	//db->DumpTables();

	// Swap in the true moving zone. Names become misleading because contents swap:
	// tree_zone_in_extra <- the actual moving zone now in extra tree
	// main_tree_zone_from <- the "from" scaffold now in main tree, to be killed by inversion
	db->XTreeDatabase::SwapTreeToTree( main_tree_zone_from, fixups,
		    						   tree_zone_in_extra, vector<TreeZone *>() );

	// ------------------------- Add "To" scaffolding patch to layout for inversion ---------------------------
	// tree_zone_in_extra now contains the moving zone	
	FreeZone scaffold_zone_to = sops->CreateSimilarScaffoldZone(tree_zone_in_extra); 
	TreePtr<Node> scaffold_base_to = scaffold_zone_to.GetBaseNode();
	TRACE("\"To\" scaffold base: ")(scaffold_zone_to)("\n");

	// Rememeber the association between the "to" scaffold node and the moving zone
	FTRACE("Making map entry, \"to\" scaffold node: ")(scaffold_base_to)("\n moving zone: \n")(tree_zone_in_extra)("\n");

	MoveInfo mi{ tree_zone_in_extra, ooo_tree_patch->GetOriginators() };
	InsertSolo( moves_map.mm, make_pair(scaffold_base_to, mi) );

	// Store the scaffold in the layout so it goes into inversion as a free zone, so it survives, 
	// and ands up in the tree at the "to" location.
	auto free_patch_to = make_shared<FreePatch>( scaffold_zone_to, ooo_tree_patch->MoveChildren() );
	*ooo_patch_ptr = free_patch_to;
	
	ValidateTreeZones(db).Run(layout);
	
	// How does the scaffold not end up in the updated tree?
	// The best argument is that, after this pass, scaffold_zone_from is not
	// in any of the patches in our layout. The layout is the intended eventual 
	// contents of the update tree. So, if inversion acts correctly, scaffold_zone_from 
	// will be deleted from the tree. OTOH scaffold_zone_to will make it into
	// inversion as a free zone and enter the tree, but we're tracking it in moves_map
	// and will swap it out and delete it during MoveInPass.
}

