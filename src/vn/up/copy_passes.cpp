#include "copy_passes.hpp"

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

void ChooseCopiesPass::Run(shared_ptr<Patch> &layout)
{
	vector<shared_ptr<Patch> *> out_of_order_patches;  
	set<XLink> in_order_bases;
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreePatch>(patch);
        switch( tree_patch->GetIntent() )
        {
			case TreePatch::Intent::DEFAULT:
			InsertSolo(in_order_bases, tree_patch->GetZone()->GetBaseXLink() );
			break;
			
			case TreePatch::Intent::MOVEABLE:
			out_of_order_patches.push_back( &patch );
			break;
			
			case TreePatch::Intent::COPYABLE:
			// Ignore
			break;
		}		
    } );    

	multiset<XLink> out_of_order_bases;
    for( shared_ptr<Patch> *ooo_patch_ptr : out_of_order_patches )
    {
		auto ooo_tree_patch = dynamic_pointer_cast<TreePatch>(*ooo_patch_ptr);
		ASSERT( ooo_tree_patch );
		XLink base_xlink = ooo_tree_patch->GetZone()->GetBaseXLink();
		ASSERT( base_xlink );
		// Unlike with in-order, there can be multiple OOO.
		out_of_order_bases.insert( base_xlink );
	}

	// Process duplications first, because we wouldn't want to duplicate
	// a TZ that aliasses a TZ that has had scaffolding put in.
    for( shared_ptr<Patch> *ooo_patch_ptr : out_of_order_patches )
    {
		auto ooo_tree_patch = dynamic_pointer_cast<TreePatch>(*ooo_patch_ptr);
		ASSERT( ooo_tree_patch );
		XLink base_xlink = ooo_tree_patch->GetZone()->GetBaseXLink();
		ASSERT( base_xlink );
		ASSERT( out_of_order_bases.count(base_xlink) >= 1 ); // this one!
		if( out_of_order_bases.count(base_xlink) >= 2 || // Other out-of-orders, we should dup all but one
		    in_order_bases.count(base_xlink) >= 1 ) // An in-order, we should dup so it can be left alone
		{
			ooo_tree_patch->SetIntent( TreePatch::Intent::COPYABLE );
			out_of_order_bases.erase(out_of_order_bases.lower_bound(base_xlink));
		}
	}
}


void CopyingPass::Run(shared_ptr<Patch> &layout)
{
	INDENT("D");
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreePatch>(patch);
        if( tree_patch->GetIntent() == TreePatch::Intent::COPYABLE )
        {		
			// This TZ is aliassed by other TZs
			TRACE("Duplicating ")(patch)("\n");

			// We'll have to duplicate. Best to duplicate the OOO one so we don't have to do a move
			shared_ptr<FreePatch> new_free_patch = tree_patch->DuplicateToFree();
			patch = new_free_patch;
		} 
	} );
}

