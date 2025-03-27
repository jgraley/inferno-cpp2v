#include "fz_merge.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- FreeZoneMerger --------------------------

FreeZoneMerger::FreeZoneMerger()
{
}
    

void FreeZoneMerger::Run( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
        {
            TRACE("Parent FreeZonePatch ")(*free_patch)("\n");
            FreeZone &free_zone = free_patch->GetZone();

            FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();
            FreeZonePatch::ChildExpressionIterator it_child = free_patch->GetChildrenBegin();
            
            while( it_child != free_patch->GetChildrenEnd() )
            {
                ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch        
                if( auto child_free_patch = dynamic_pointer_cast<FreeZonePatch>(*it_child) )
                {    
                    TRACE("Child FreeZonePatch ")(*child_free_patch)(" and terminus ")(*it_t)("\n");
                    FreeZone &child_free_zone = child_free_patch->GetZone();
                    it_t = free_zone.MergeTerminus( it_t, make_unique<FreeZone>(child_free_zone) );        
                    TRACE("Mutator OK\n");
                    it_child = free_patch->SpliceOver( it_child, child_free_patch->MoveChildExpressions() );
                    TRACE("Splice OK\n");
                }    
                else
                {
                    TRACE("Child TreeZonePatch: SKIPPING and terminus ")(*it_t)("\n");
                    it_t++;
                    it_child++;
                }                        
            } 
            ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch    
            TRACE("Loop OK\n");
        }
    } );            
}


void FreeZoneMerger::Check( shared_ptr<Patch> &layout )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
        {
            free_patch->ForChildren([&](shared_ptr<Patch> &child_patch)
            {
                if( dynamic_pointer_cast<FreeZonePatch>(child_patch) )
                    ASSERT(false)("Free zone ")(*patch)(" touching another free zone ")(*child_patch);
            } );
        }
    } );        
}
