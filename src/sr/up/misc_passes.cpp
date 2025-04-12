#include "misc_passes.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- ToMutablePass --------------------------

ToMutablePass::ToMutablePass(XTreeDatabase *db_) :
    db(db_)
{
}
    

void ToMutablePass::Run( shared_ptr<Patch> &layout )
{
    TreeZonePatch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreeZonePatch> &tree_patch)
    {
		TreeZone *zone = tree_patch->GetZone();
        if( !dynamic_cast<MutableTreeZone *>(zone) )
		{			
			auto mutable_zone = db->MakeMutableTreeZone( zone->GetBaseXLink(),
														 zone->GetTerminusXLinks() );
			tree_patch->SetZone(move(mutable_zone));                                			
        }
    } );    
}

// ------------------------- ProtectDEPass --------------------------


ProtectDEPass::ProtectDEPass(const XTreeDatabase *db_) :
    db(db_)
{
}
 
 
void ProtectDEPass::Run( shared_ptr<Patch> &layout )
{
    TreeZoneRelation tz_relation( db );

    auto extra_root_xlinks = db->GetExtraRootXLinks();
    for( XLink xlink : extra_root_xlinks )
    {
        auto extra_tree = XTreeZone::CreateSubtree(xlink);
        Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &r_patch)
        {
			// Filter manually because we'll change the type
            if( auto right_tree_patch = dynamic_pointer_cast<TreeZonePatch>(r_patch) )
            {            
                auto p = tz_relation.CompareHierarchical( *extra_tree, *right_tree_patch->GetZone() );
                if( p.second == ZoneRelation::OVERLAP_GENERAL || 
                    p.second == ZoneRelation::OVERLAP_TERMINII ||
                    p.second == ZoneRelation::EQUAL )
                {
                    r_patch = right_tree_patch->DuplicateToFree();
                }
            }
        });
    }    
}


// ------------------------- MarkersPass --------------------------

MarkersPass::MarkersPass( const XTreeDatabase *db_ ) :
    db( db_ )
{
}


void MarkersPass::Run( shared_ptr<Patch> &layout )
{
    TreeZoneRelation tz_relation( db );
    
    list<RequiresSubordinateSCREngine *> markers;

    Patch::ForDepthFirstWalk( layout, [&](shared_ptr<Patch> &patch) // Act on wind-in
    {
        if( true )
        {    
            // Append to our list
            markers.splice( markers.end(), patch->GetEmbeddedMarkers() );
            patch->ClearEmbeddedMarkers();
            
            if( !patch->GetZone()->IsEmpty() )
            {
                for( RequiresSubordinateSCREngine *agent : markers )
                    patch->GetZone()->MarkBaseForEmbedded(agent);
                markers.clear();
            }
            // If zone is empty, it has one child, which we will meet at the next iteration
            // of the depth-first walk. That's the one where we should enact the marker. 
            // Unless it's empty too, in which case repeat - we must find non-empty eventually!
        }
    }, nullptr );
    
    ASSERT( markers.empty() ); // could not place marker because we saw only empty zones.
}

// ------------------------- DuplicateAllPass --------------------------

DuplicateAllPass::DuplicateAllPass()
{
}
    

void DuplicateAllPass::Run( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
		// Filter manually because we'll change the type
        if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
        {
            patch = tree_patch->DuplicateToFree();
        }
    } );    
}


void DuplicateAllPass::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        ASSERT( dynamic_pointer_cast<FreeZonePatch>(patch) );
    } );    
}

// ------------------------- EmptyZonePass --------------------------

EmptyZonePass::EmptyZonePass()
{
}
    

void EmptyZonePass::Run( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
		// Filter manually because may change patch type
        if( auto zone_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
        {
            if( zone_patch->GetZone()->IsEmpty() )
            {            
                shared_ptr<Patch> child_patch = OnlyElementOf( zone_patch->GetChildren() );
                child_patch->AddEmbeddedMarkers( zone_patch->GetEmbeddedMarkers() );
                patch = child_patch;
            }
        }
    } );    
}


void EmptyZonePass::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        ASSERT( !patch->GetZone()->IsEmpty() )("Found empty zone in populate op: ")(patch->GetZone());
    } );    
}

