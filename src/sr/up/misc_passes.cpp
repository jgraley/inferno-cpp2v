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
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreePatch> &tree_patch)
	{
		DBCommon::TreeOrdinal ordinal = tree_patch->GetZone()->GetTreeOrdinal();
		switch( db->GetTreeType( ordinal ) )
		{
			case DBCommon::TreeType::DOMAIN_EXTENSION:
				// Indicate we won't be moving it
				tree_patch->SetIntent( TreePatch::Intent::COPYABLE );
				break;
				
			case DBCommon::TreeType::MAIN:
				break;
				
			case DBCommon::TreeType::UPDATE:
				ASSERTFAIL();
		}
	});
}

// ------------------------- EmptyZonePass --------------------------

EmptyZonePass::EmptyZonePass()
{
}
    

void EmptyZonePass::Run( shared_ptr<Patch> &layout )
{
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreePatch>(patch);
        if( tree_patch->GetZone()->IsEmpty() )
        {            
			// Child could be tree or free
			shared_ptr<Patch> child_patch = SoloElementOf( tree_patch->GetChildren() );
			child_patch->AddOriginators( tree_patch->GetOriginators() );
			patch = child_patch; 
		}
    } );    
}


void EmptyZonePass::Check( shared_ptr<Patch> &layout )
{
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreePatch> &tree_patch)
    {
        ASSERT( !tree_patch->GetZone()->IsEmpty() )("Found empty tree zone: ")(tree_patch->GetZone());
    } );    
}

// ------------------------- MarkersPass --------------------------

void MarkersPass::Run( shared_ptr<Patch> &layout )
{   
    list<PatternLink> originators;

    Patch::ForDepthFirstWalk( layout, [&](shared_ptr<Patch> &patch) // Act on wind-in
    {
		// Append to our list
		originators.splice( originators.end(), patch->GetOriginators() );
		patch->ClearOriginators();
		
		if( !patch->GetZone()->IsEmpty() )
		{
			for( PatternLink plink : originators )
				plink.GetChildAgent()->MarkReplaceKey( patch->GetZone()->GetBaseNode() );    
			originators.clear();
		}
		// If zone is empty, it has one child, which we will meet at the next iteration
		// of the depth-first walk. That's the one where we should enact the marker. 
		// Unless it's empty too, in which case repeat - we must find non-empty eventually!
    }, nullptr );
    
    ASSERT( originators.empty() ); // could not place marker because we saw only empty zones.
}

// ------------------------- DuplicateAllPass --------------------------

void DuplicateAllPass::Run( shared_ptr<Patch> &layout )
{
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        auto tree_patch = dynamic_pointer_cast<TreePatch>(patch);
        patch = tree_patch->DuplicateToFree();
    } );    
}


void DuplicateAllPass::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        ASSERT( dynamic_pointer_cast<FreePatch>(patch) );
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
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreePatch> &patch)
    {
		patch->GetZone()->Validate(db);
    } );    
}
// ------------------------- SetTreeOrdinals --------------------------

SetTreeOrdinals::SetTreeOrdinals(const XTreeDatabase *db_) :
    db(db_)
{
}

 
void SetTreeOrdinals::Run( shared_ptr<Patch> layout )
{
    TreePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<TreePatch> &patch)
    {
		TreeZone *zone = patch->GetZone();
		zone->SetTreeOrdinal( db->GetTreeOrdinalFor( zone->GetBaseXLink() ) );
    } );    
}
