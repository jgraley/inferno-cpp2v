#include "fz_merge.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- FreeZoneMergeImpl --------------------------  

void FreeZoneMergeImpl::Run( shared_ptr<Patch> &layout, PolicyFunction policy )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
        {
            TRACE("Parent FreeZonePatch ")(*free_patch)("\n");
            FreeZone *free_zone = free_patch->GetZone();

            FreeZone::TerminusIterator it_t = free_zone->GetTerminiiBegin();
            FreeZonePatch::ChildExpressionIterator it_child = free_patch->GetChildrenBegin();
            
            while( it_child != free_patch->GetChildrenEnd() )
            {
                ASSERT( it_t != free_zone->GetTerminiiEnd() ); // length mismatch        
                if( auto child_free_patch = dynamic_pointer_cast<FreeZonePatch>(*it_child) )
                {    
                    TRACE("Child FreeZonePatch ")(*child_free_patch)(" and terminus ")(*it_t)("\n");
                    FreeZone *child_free_zone = child_free_patch->GetZone();                    
                    if( policy(free_zone, child_free_zone) )
					{
                        it_t = free_zone->MergeTerminus( it_t, make_unique<FreeZone>(*child_free_zone) );  // TODO why make_unique here?      
						TRACE("Mutator OK\n");
						it_child = free_patch->SpliceOver( it_child, child_free_patch->MoveChildExpressions() );
						TRACE("Splice OK\n");
						continue;
					}
				}
				
                TRACE("Child TreeZonePatch: SKIPPING and terminus ")(*it_t)("\n");
                it_t++;
                it_child++;                                       
            } 
            ASSERT( it_t == free_zone->GetTerminiiEnd() ); // length mismatch    
            TRACE("Loop OK\n");
        }
    } );            
}


void FreeZoneMergeImpl::Check( shared_ptr<Patch> &layout, PolicyFunction policy )
{
    Patch::ForDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
        if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
        {
            free_patch->ForChildren([&](shared_ptr<Patch> &child_patch)
            {
                if( auto child_free_patch = dynamic_pointer_cast<FreeZonePatch>(child_patch) )
                    ASSERT(!policy(child_free_patch->GetZone(), free_patch->GetZone()))
                          ("Free patch:")(*patch)(" touching another free patch ")(*child_patch);
            } );
        }
    } );        
}

// ------------------------- FreeZoneMerger --------------------------

void FreeZoneMerger::Run( shared_ptr<Patch> &layout )
{
	impl.Run(layout, bind(&FreeZoneMerger::Policy, this, placeholders::_1, placeholders::_2));
}
    
    
void FreeZoneMerger::Check( shared_ptr<Patch> &layout )
{
	impl.Check(layout, bind(&FreeZoneMerger::Policy, this, placeholders::_1, placeholders::_2));
}


bool FreeZoneMerger::Policy(const FreeZone *, const FreeZone *) const
{
	return true;
}

// ------------------------- FreeZoneMergeCollectionBases --------------------------

void FreeZoneMergeCollectionBases::Run( shared_ptr<Patch> &layout )
{
	//FTRACE(layout);
	impl.Run(layout, bind(&FreeZoneMergeCollectionBases::Policy, this, placeholders::_1, placeholders::_2));
}
    
    
void FreeZoneMergeCollectionBases::Check( shared_ptr<Patch> &layout )
{
	// Check is stricter: no container bases anywhere, regardless of parent type
	Patch::ForDepthFirstWalk( layout, [&](shared_ptr<Patch> &patch)
	{
		if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
			ASSERT( !free_patch->GetZone()->TryGetContainerBase() )(free_patch);
	}, nullptr );
}


bool FreeZoneMergeCollectionBases::Policy(const FreeZone *zone, const FreeZone *child_zone) const
{	
	//FTRACE("Zone: ")(*zone)("\nhas child: ")(*child_zone)("\n");
	return !!child_zone->TryGetContainerBase();
}

