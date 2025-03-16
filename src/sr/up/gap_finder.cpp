#include "gap_finder.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneGapFinder::TreeZoneGapFinder( XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneGapFinder::Run(shared_ptr<Patch> layout)
{	
	gaps.clear();
	
	// Gather all the tree zones from the patches in the source layout, and
	// put them in a depth-first ordering on base (i.e. ordered relative to CURRENT tree)
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto tz_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
		{				
			CheckPatch(tz_patch);
		}
	} );
}


const TreeZoneGapFinder::GapSet &TreeZoneGapFinder::GetGaps() const
{
	return gaps;
}


void TreeZoneGapFinder::CheckPatch(shared_ptr<TreeZonePatch> patch)
{
	TreeZone tz = patch->GetZone();
	TreeZone::TerminusIterator it_t = tz.GetTerminiiBegin();
	patch->ForChildren( [&](shared_ptr<Patch> &child_patch)	
	{
		XLink terminus_xlink = *it_t++; // inclusive (terminus XLink equals base XLink of attached tree zone)
		if( auto child_tz_patch = dynamic_pointer_cast<TreeZonePatch>(child_patch) ) // Child IS a tree zone...
		{
			XLink child_base_xlink = child_tz_patch->GetZone().GetBaseXLink();
			if( child_base_xlink != terminus_xlink )                           // ...but not directly attached in current tree
				gaps.insert( make_pair( terminus_xlink, child_base_xlink ) );
		}
	} );
}

