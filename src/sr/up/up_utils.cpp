#include "up_utils.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- EmptyZoneElider --------------------------

EmptyZoneElider::EmptyZoneElider()
{
}
	

void EmptyZoneElider::Run( shared_ptr<Patch> &layout )
{
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto pz_op = dynamic_pointer_cast<ZonePatch>(patch) )
            if( pz_op->GetZone().IsEmpty() )
            {
				shared_ptr<Patch> child_patch = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<ZonePatch>(child_patch) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				patch = child_patch;
			}
	} );	
}


void EmptyZoneElider::Check( shared_ptr<Patch> &layout )
{
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto pz_op = dynamic_pointer_cast<ZonePatch>(patch) )
            ASSERT( !pz_op->GetZone().IsEmpty() )("Found empty zone in populate op: ")(pz_op->GetZone());
	} );	
}

// ------------------------- BaseForEmbeddedMarkPropagation --------------------------

BaseForEmbeddedMarkPropagation::BaseForEmbeddedMarkPropagation( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void BaseForEmbeddedMarkPropagation::Run( shared_ptr<Patch> &layout )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at TreeZonePatch patches
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto pz_op = dynamic_pointer_cast<ZonePatch>(patch) )
		{	
			for( RequiresSubordinateSCREngine *agent : pz_op->GetEmbeddedMarkers() )
			{
				pz_op->GetZone().MarkBaseForEmbedded(agent);
			}
			pz_op->ClearEmbeddedMarkers();
		}
	} );
}

// ------------------------- DuplicateAllToFree --------------------------

DuplicateAllToFree::DuplicateAllToFree()
{
}
	

void DuplicateAllToFree::Run( shared_ptr<Patch> &layout )
{
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
        {
			patch = tree_patch->DuplicateToFree();
		}
	} );	
}


void DuplicateAllToFree::Check( shared_ptr<Patch> &layout )
{
	Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
	{
		ASSERT( dynamic_pointer_cast<FreeZonePatch>(patch) );
	} );	
}


