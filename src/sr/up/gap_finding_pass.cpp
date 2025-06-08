#include "gap_finding_pass.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

GapFindingPass::GapFindingPass()
{
}


void GapFindingPass::Run(shared_ptr<Patch> layout)
{	
	INDENT("G");
	TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreeZonePatch> &tree_patch)
	{
		CheckPatch(tree_patch);
	} );
}


void GapFindingPass::CheckPatch(shared_ptr<TreeZonePatch> patch)
{
	if( patch->GetIntent() != TreeZonePatch::Intent::DEFAULT )
		return;
	TreeZone *tz = patch->GetZone();
	size_t index = 0;
	Patch::ForChildren( patch, [&](shared_ptr<Patch> &child_patch)	
	{
		XLink terminus_xlink = tz->GetTerminusXLink(index++); // inclusive (terminus XLink equals base XLink of attached tree zone)
		if( auto child_tz_patch = dynamic_pointer_cast<TreeZonePatch>(child_patch) ) // Child IS a tree zone...
		{
			if( child_tz_patch->GetIntent() != TreeZonePatch::Intent::DEFAULT )
				return; // Acts like continue
				
			XLink child_base_xlink = child_tz_patch->GetZone()->GetBaseXLink();
			if( child_base_xlink != terminus_xlink )    // ...but not directly attached in current tree
			{				
				// Action: insert an empty free zone
				auto new_patch = make_shared<FreeZonePatch>( FreeZone::CreateEmpty(), 
				                                             list<shared_ptr<Patch>>{ child_patch } );
				child_patch = new_patch;
			}
		}
	} );
}

