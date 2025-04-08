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
        if( auto zone_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
        {
            if( zone_patch->GetZone()->IsEmpty() )
            {            
                shared_ptr<Patch> child_patch = OnlyElementOf( zone_patch->GetChildExpressions() );
                if( auto child_zone_patch = dynamic_pointer_cast<ZonePatch>(child_patch) )
                    child_zone_patch->AddEmbeddedMarkers( zone_patch->GetEmbeddedMarkers() );
                patch = child_patch;
            }
        }
    } );    
}


void EmptyZoneElider::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto free_patch = dynamic_pointer_cast<ZonePatch>(patch) )
            ASSERT( !free_patch->GetZone()->IsEmpty() )("Found empty zone in populate op: ")(free_patch->GetZone());
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
    
    list<RequiresSubordinateSCREngine *> markers;

    Patch::ForDepthFirstWalk( layout, [&](shared_ptr<Patch> &patch) // Act on wind-in
    {
        auto zone_patch = dynamic_pointer_cast<ZonePatch>(patch);
        ASSERT( zone_patch );
        if( true )
        {    
            // Append to our list
            markers.splice( markers.end(), zone_patch->GetEmbeddedMarkers() );
            zone_patch->ClearEmbeddedMarkers();
            
            if( !zone_patch->GetZone()->IsEmpty() )
            {
                for( RequiresSubordinateSCREngine *agent : markers )
                    zone_patch->GetZone()->MarkBaseForEmbedded(agent);
                markers.clear();
            }
            // If zone is empty, it has one child, which we will meet at the next iteration
            // of the depth-first walk. That's the one where we should enact the marker. 
            // Unless it's empty too, in which case repeat - we must find non-empty eventually!
        }
    }, nullptr );
    
    ASSERT( markers.empty() ); // could not place marker because we saw only empty zones.
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

// ------------------------- TreeZonesToMutable --------------------------

TreeZonesToMutable::TreeZonesToMutable(XTreeDatabase *db_) :
    db(db_)
{
}
    

void TreeZonesToMutable::Run( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto zone_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
        {
			TreeZone *zone = zone_patch->GetZone();
            if( !dynamic_cast<MutableTreeZone *>(zone) )
			{			
				auto mutable_zone = db->MakeMutableTreeZone( zone->GetBaseXLink(),
															 zone->GetTerminusXLinks() );
				zone_patch->SetZone(move(mutable_zone));                                
			}
        }
    } );    
}

