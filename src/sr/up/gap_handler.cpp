#include "gap_handler.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneGapHandler::TreeZoneGapHandler()
{
}


void TreeZoneGapHandler::Run(shared_ptr<Patch> layout)
{	
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto tz_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
		{				
			CheckPatch(tz_patch);
		}
	} );
}


void TreeZoneGapHandler::CheckPatch(shared_ptr<TreeZonePatch> patch)
{
	TreeZone tz = patch->GetZone();
	TreeZone::TerminusIterator it_t = tz.GetTerminiiBegin();
	patch->ForChildren( [&](shared_ptr<Patch> &child_patch)	
	{
		XLink terminus_xlink = *it_t++; // inclusive (terminus XLink equals base XLink of attached tree zone)
		if( auto child_tz_patch = dynamic_pointer_cast<TreeZonePatch>(child_patch) ) // Child IS a tree zone...
		{
			XLink child_base_xlink = child_tz_patch->GetZone().GetBaseXLink();
			if( child_base_xlink != terminus_xlink )    // ...but not directly attached in current tree
			{				
				// Action: insert an empty free zone
				auto new_patch = make_shared<FreeZonePatch>( FreeZone::CreateEmpty(), list<shared_ptr<Patch>>{ child_patch } );
				child_patch = new_patch;
			}
		}
	} );
}

