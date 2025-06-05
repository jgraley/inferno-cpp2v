#include "alt_ordering_checker.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "misc_passes.hpp"

#include <iostream>

using namespace SR;

AltOrderingChecker::AltOrderingChecker(const XTreeDatabase *db_) :
    db( db_ ),
    dfr( db )
{
}
    

void AltOrderingChecker::Check( shared_ptr<Patch> layout )
{
    Worker(layout, db->GetMainRootXLink(), false);
}


void AltOrderingChecker::Worker( shared_ptr<Patch> patch, XLink x_sub_base, bool should_touch )
{	
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
    {
        INDENT("t");
		TRACE("Checking x_sub_base: ")(x_sub_base)(" against patch:\n")(patch)("\nshould_touch: ")(should_touch)("\n");

        // Got a Tree Zone - check ordering of its base, strictness depending on who called us
        const XTreeZone *tree_zone = tree_patch->GetZone();
        XLink base = tree_zone->GetBaseXLink();
        auto p = dfr.CompareHierarchical( x_sub_base, base );
        if( should_touch )
			ASSERT( p.second==DepthFirstRelation::EQUAL )(x_sub_base)(" vs ")(base)(" got ")(p);
		else
			ASSERT( p.second==DepthFirstRelation::EQUAL ||
			        p.second==DepthFirstRelation::LEFT_IS_ANCESTOR )(x_sub_base)(" vs ")(base)(" got ")(p);
        
        if( tree_zone->IsEmpty() )
		{
			TRACE("TZ is empty\n");
			ASSERT( base==SoloElementOf(tree_zone->GetTerminusXLinks())); // Definition of empty tree zone
			Worker(SoloElementOf(patch->GetChildren()), SoloElementOf(tree_zone->GetTerminusXLinks()), true);
			return;
		}
        
        // TODO this loop is similar to the one in the FZ case. Pop them both out into 
        // a new method. First call tree_patch->GetChildren() to get a list<shared_ptr<Patch>>
        // and co-loop with that. Then pop out into new function. Then have 
        // AppendNextDescendantTreePatches() also use a list. Then the FZ case 
        // populates vector<XLink> with the ndt_bases and use the new method.
        // Prefactor: combine the two loops in FZ case.
        // Co-loop over the children/terminii
		XLink prev = XLink();
        vector<XLink> terminii = tree_zone->GetTerminusXLinks();
        FreeZonePatch::ChildPatchIterator it_child = tree_patch->GetChildrenBegin();        
        for( XLink terminus : terminii )
        {
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch

			if( prev )
			{
				auto p = dfr.CompareHierarchical( prev, terminus );
				ASSERT( p.first < 0 ); // strict: no repeated XLinks
				// Terminii should not be in parent/child relationships
				ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR )(prev)(" vs ")(terminus)(" got ")(p);
			}

			auto p2 = dfr.CompareHierarchical( base, terminus );
			ASSERT( p2.second == DepthFirstRelation::LEFT_IS_ANCESTOR )(base)(" vs ")(terminus)(" got ")(p2); 
				
			prev = terminus;        

            // Check everything under the corresponding child is in order. 
            Worker( *it_child, terminus, false );
            
            ++it_child;
        }
        ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
    {
        INDENT("f");
		TRACE("Checking x_sub_base: ")(x_sub_base)(" against patch:\n")(patch)("\nshould_touch: ")(should_touch)("\n");
        ASSERT( !free_patch->GetZone()->IsEmpty() ); // TODO handle this case as with TZs, above
        
		vector<shared_ptr<TreeZonePatch>> ndt_patches;
		AppendNextDescendantTreePatches( patch, &ndt_patches );
		// We're finding tree patches, so they will all have XLinks as bases.
		
		TRACE("TZ patches found by AppendNextDescendantTreePatches():\n", ndt_patches.size())(ndt_patches)("\n");
		
		XLink prev = XLink();
        for( shared_ptr<TreeZonePatch> ndt_patch : ndt_patches )
        {
			// ndt_base would become a terminus during inversion, so must
			// obey the same rules as TZ terminii, relative to sub_base
			XLink ndt_base = ndt_patch->GetZone()->GetBaseXLink();
			TRACE("Checking ndt_base: ")(ndt_base)("\n");
			
			if( prev )
			{
				auto p = dfr.CompareHierarchical( prev, ndt_base );
				ASSERT( p.first < 0 )(prev)(" vs ")(ndt_base)(" got ")(p); // strict: no repeated XLinks
				// Terminii should not be in parent/child relationships
				ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR )(prev)(" vs ")(ndt_base)(" got ")(p); 
			}

			auto p2 = dfr.CompareHierarchical( x_sub_base, ndt_base );
			ASSERT( p2.second == DepthFirstRelation::LEFT_IS_ANCESTOR ||
			        p2.second == DepthFirstRelation::EQUAL )(x_sub_base)(" vs ")(ndt_base)(" got ")(p2);  

			prev = ndt_base;        
		}				

        for( shared_ptr<TreeZonePatch> ndt_patch : ndt_patches )
        {
			XLink ndt_base = ndt_patch->GetZone()->GetBaseXLink();
            Worker( ndt_patch, ndt_base, false );
        }
    }
    else
    {
        ASSERTFAIL();
    }
}


void AltOrderingChecker::AppendNextDescendantTreePatches( shared_ptr<Patch> start_patch, 
                                                          vector<shared_ptr<TreeZonePatch>> *ndt_patches )
{
    // Insert descendent tree zones, skipping over free zones, into a list for
    // convenience.
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(start_patch) )
    {
        ndt_patches->push_back( tree_patch );
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(start_patch) )
    {
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)
        {
            AppendNextDescendantTreePatches( child_patch, ndt_patches );
        } );
    }
    else
    {
        ASSERTFAIL();
    }	
}                                                          


