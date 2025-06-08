#include "merge_passes.hpp"

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
	INDENT("F");
    FreePatch::ForFreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<FreePatch> &free_patch)
    {
		TRACE("Parent FreePatch ")(*free_patch)("\n");
		FreeZone *free_zone = free_patch->GetZone();

		FreeZone::TerminusIterator it_t = free_zone->GetTerminiiBegin();
		FreePatch::ChildPatchIterator it_child = free_patch->GetChildrenBegin();
		
		while( it_child != free_patch->GetChildrenEnd() )
		{
			ASSERT( it_t != free_zone->GetTerminiiEnd() ); // length mismatch        
			if( auto child_free_patch = dynamic_pointer_cast<FreePatch>(*it_child) )
			{    
				TRACE("Child FreePatch ")(*child_free_patch)(" and terminus ")(*it_t)("\n");
				FreeZone *child_free_zone = child_free_patch->GetZone();                    
				if( policy(free_zone, child_free_zone) )
				{
					it_t = free_zone->MergeTerminus( it_t, make_unique<FreeZone>(*child_free_zone) );  // TODO why make_unique here?      
					TRACE("Mutator OK\n");
					it_child = free_patch->SpliceOver( it_child, child_free_patch->MoveChildren() );
					TRACE("Splice OK\n");
					continue;
				}
			}
			
			TRACE("Child TreePatch: SKIPPING and terminus ")(*it_t)("\n");
			it_t++;
			it_child++;                                       
		} 
		ASSERT( it_t == free_zone->GetTerminiiEnd() ); // length mismatch    
		TRACE("Loop OK\n");
    } );            
}


void FreeZoneMergeImpl::Check( shared_ptr<Patch> &layout, PolicyFunction policy )
{
    FreePatch::ForFreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<FreePatch> &free_patch)
    {
        FreePatch::ForFreeChildren( free_patch, [&](shared_ptr<FreePatch> &child_free_patch)
        {
            ASSERT(!policy(child_free_patch->GetZone(), free_patch->GetZone()))
                  ("Free patch:")(*free_patch)(" touching another free patch ")(*child_free_patch);
        } );
    } );        
}

// ------------------------- MergeFreesPass --------------------------

void MergeFreesPass::Run( shared_ptr<Patch> &layout )
{
	impl.Run(layout, bind(&MergeFreesPass::Policy, this, placeholders::_1, placeholders::_2));
}
    
    
void MergeFreesPass::Check( shared_ptr<Patch> &layout )
{
	impl.Check(layout, bind(&MergeFreesPass::Policy, this, placeholders::_1, placeholders::_2));
}


bool MergeFreesPass::Policy(const FreeZone *, const FreeZone *) const
{
	return true;
}

// ------------------------- MergeWidesPass --------------------------

void MergeWidesPass::Run( shared_ptr<Patch> &layout )
{
	INDENT("W");
	//FTRACE(layout);
	impl.Run(layout, bind(&MergeWidesPass::Policy, this, placeholders::_1, placeholders::_2));
}
    
    
void MergeWidesPass::Check( shared_ptr<Patch> &layout )
{
	// Check is stricter: no container bases anywhere, regardless of parent type
	FreePatch::ForFreeDepthFirstWalk( layout, [&](shared_ptr<FreePatch> &free_patch)
	{
		ASSERT( !free_patch->GetZone()->TryGetContainerBase() )(free_patch);
	}, nullptr );
}


bool MergeWidesPass::Policy(const FreeZone *zone, const FreeZone *child_zone) const
{	
	TRACE("Zone: ")(*zone)("\nhas child: ")(*child_zone)("\n");
	return !!child_zone->TryGetContainerBase();
}

