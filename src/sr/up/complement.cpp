#include "complement.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneComplementer::TreeZoneComplementer( XTreeDatabase *db_ ) :
	db( db_ ),
	df_rel( db ),
	source_tzs_df_by_base( df_rel )	
{
}


void TreeZoneComplementer::Run(XLink target_origin, shared_ptr<Patch> source_layout)
{	
	// Gather all the tree zones from the patches in the source layout, and
	// put them in a depth-first ordering on base (i.e. ordered relative to CURRENT tree)
	Patch::ForDepthFirstWalk( source_layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto tz_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
		{	
			const TreeZone &tz = tz_patch->GetZone();
			
			source_tzs_df_by_base.emplace( tz.GetBaseXLink(), tz );
		}
	} );
	
	WalkTreeZones(target_origin);
}


void TreeZoneComplementer::WalkTreeZones(XLink target_base)
{
	if( source_tzs_df_by_base.count(target_base) > 0 )
	{
		// There's a TZ in the layout, so recurse.
		const TreeZone &found_tz = source_tzs_df_by_base.at(target_base);
		for( XLink terminus : found_tz.GetTerminusXLinks() )
			WalkTreeZones( terminus );

	}
	else
	{
		// make a complement TZ and store it.
		CreateComplementTZ(target_base);
		
	}

}

void TreeZoneComplementer::CreateComplementTZ(XLink target_base)
{
	ASSERT( source_tzs_df_by_base.count(target_base) == 0 );
	
	vector<XLink> complement_terminii;
	
	DepthFirstOrderingZones::const_iterator it = source_tzs_df_by_base.lower_bound(target_base);
	// The first element in the ordering which is not ordered before target_base.
	// target_base is not in there, so we're at first child if there are any.
	// Loop through children	
	while( df_rel.CompareHierarchical(target_base, it->first).second == DepthFirstRelation::RelType::LEFT_IS_ANCESTOR )
	{
		XLink child = it->first;
	
		// On the unwind now
		complement_terminii.push_back( child );

		// Recurse on children
		WalkTreeZones( child );

		++it;
		// Loop PAST any children of this child
		while( df_rel.CompareHierarchical(child, it->first).second == DepthFirstRelation::RelType::LEFT_IS_ANCESTOR )
			++it;
	}
	
	complement.push_back( TreeZone( target_base, complement_terminii ) );
}

