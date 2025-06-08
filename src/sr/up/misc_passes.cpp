#include "misc_passes.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- ProtectDEPass --------------------------

ProtectDEPass::ProtectDEPass(const XTreeDatabase *db_) :
    db(db_)
{
}
 
 
void ProtectDEPass::Run( shared_ptr<Patch> &layout )
{
    TreeZoneRelation tz_relation( db );

    auto extra_root_ordinals = db->GetExtraRootOrdinals();
    for( DBCommon::TreeOrdinal ordinal : extra_root_ordinals )
    {
        auto extra_tree = TreeZone::CreateSubtree(db->GetRootXLink(ordinal));
        
        TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &r_patch)
        {
			// Filter manually because we'll change the type
            auto right_tree_patch = dynamic_pointer_cast<TreeZonePatch>(r_patch);
			auto p = tz_relation.CompareHierarchical( extra_tree, *right_tree_patch->GetZone() );
			if( p.second == ZoneRelation::OVERLAP_GENERAL || 
				p.second == ZoneRelation::OVERLAP_TERMINII ||
				p.second == ZoneRelation::EQUAL )
			{
				// First set the correct ordinal on the tree zone
				right_tree_patch->GetZone()->SetTreeOrdinal( ordinal );
				
				// Then indicate we won't be moving it
				right_tree_patch->SetIntent( TreeZonePatch::Intent::COPYABLE );				
            }
        });
    }    
}

// ------------------------- EmptyZonePass --------------------------

EmptyZonePass::EmptyZonePass()
{
}
    

void EmptyZonePass::Run( shared_ptr<Patch> &layout )
{
    TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch);
        if( tree_patch->GetZone()->IsEmpty() )
        {            
			// Child could be tree or free
			shared_ptr<Patch> child_patch = SoloElementOf( tree_patch->GetChildren() );
			child_patch->AddEmbeddedMarkers( tree_patch->GetEmbeddedMarkers() );
			patch = child_patch; 
		}
    } );    
}


void EmptyZonePass::Check( shared_ptr<Patch> &layout )
{
    TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreeZonePatch> &tree_patch)
    {
        ASSERT( !tree_patch->GetZone()->IsEmpty() )("Found empty tree zone: ")(tree_patch->GetZone());
    } );    
}

// ------------------------- MarkersPass --------------------------

void MarkersPass::Run( shared_ptr<Patch> &layout )
{   
    list<RequiresSubordinateSCREngine *> markers;

    Patch::ForDepthFirstWalk( layout, [&](shared_ptr<Patch> &patch) // Act on wind-in
    {
		// Append to our list
		markers.splice( markers.end(), patch->GetEmbeddedMarkers() );
		patch->ClearEmbeddedMarkers();
		
		if( !patch->GetZone()->IsEmpty() )
		{
			for( RequiresSubordinateSCREngine *agent : markers )
				agent->MarkOriginForEmbedded( patch->GetZone()->GetBaseNode() );    
			markers.clear();
		}
		// If zone is empty, it has one child, which we will meet at the next iteration
		// of the depth-first walk. That's the one where we should enact the marker. 
		// Unless it's empty too, in which case repeat - we must find non-empty eventually!
    }, nullptr );
    
    ASSERT( markers.empty() ); // could not place marker because we saw only empty zones.
}

// ------------------------- DuplicateAllPass --------------------------

void DuplicateAllPass::Run( shared_ptr<Patch> &layout )
{
    TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch);
        patch = tree_patch->DuplicateToFree();
    } );    
}


void DuplicateAllPass::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        ASSERT( dynamic_pointer_cast<FreeZonePatch>(patch) );
    } );    
}

// ------------------------- ScaffoldChecker --------------------------

void ScaffoldChecker::Run( shared_ptr<Patch> layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
		Check(patch); 
    } );    
}

void ScaffoldChecker::Check(shared_ptr<Patch> patch)
{
	Zone *zone = patch->GetZone();
	string tn = zone->GetBaseNode()->GetTypeName();
	ASSERT( tn.find("Scaffold") == string::npos )(tn);
}

// ------------------------- ValidateTreeZones --------------------------

ValidateTreeZones::ValidateTreeZones(const XTreeDatabase *db_) :
    db(db_)
{
}
 
void ValidateTreeZones::Run( shared_ptr<Patch> layout )
{
    TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreeZonePatch> &patch)
    {
		patch->GetZone()->Validate(db);
    } );    
}
