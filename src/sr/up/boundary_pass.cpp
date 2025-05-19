#include "boundary_pass.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

BoundaryPass::BoundaryPass( XTreeDatabase *db_ ) :
    db( db_ ),
    dfr( db ),
    boundaries( dfr )
{
}


void BoundaryPass::Run(shared_ptr<Patch> &layout)
{	
	INDENT("B");
	boundaries.clear();
    TreeZonePatch::ForTreeDepthFirstWalk( layout, nullptr, [&](shared_ptr<Patch> &patch)
    {
		auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch);
		GatherBoundaries(tree_patch->GetZone());
		check_queue.push(&patch);
	} );

	TRACE("Boundaries:")(boundaries)("\n");
	TRACE("Check queue:")(check_queue)("\n");

    while( !check_queue.empty() ) 
    {
		CheckTreeZoneAtBoundaries(check_queue.front());
		check_queue.pop();
	}
}


void BoundaryPass::GatherBoundaries(TreeZone *tree_zone)
{
	boundaries.insert( tree_zone->GetBaseXLink() );
	for( XLink terminus : tree_zone->GetTerminusXLinks() )
	{
		boundaries.insert(terminus);
	} 
}


void BoundaryPass::CheckTreeZoneAtBoundaries( shared_ptr<Patch> *patch_ptr )
{
	TRACE("Checking ")(*patch_ptr)("\n");
	auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*patch_ptr);
	TreeZone *tree_zone = tree_patch->GetZone();
	XLink lower_excl = tree_zone->GetBaseXLink();	
	
	for( XLink terminus: tree_zone->GetTerminusXLinks() )
	{
		XLink upper_excl = terminus;
		
		XLink boundary_in_interior = TryGetBoundaryInRange( lower_excl, false, upper_excl, false );
		if( boundary_in_interior )
		{
			SplitTreeZoneAtXLink( patch_ptr, boundary_in_interior );
			return; 
		}
		
		lower_excl = XTreeDatabase::GetLastDescendantXLink(terminus);
	}
	
	XLink upper_incl = XTreeDatabase::GetLastDescendantXLink(tree_zone->GetBaseXLink());

	XLink boundary_in_interior = TryGetBoundaryInRange( lower_excl, false, upper_incl, true );
	if( boundary_in_interior )
		SplitTreeZoneAtXLink( patch_ptr, boundary_in_interior );
}


XLink BoundaryPass::TryGetBoundaryInRange( XLink lower, bool lower_incl, XLink upper, bool upper_incl )
{
	TRACE("Looking for boundary ")(lower_incl?">=":">")(lower)("; ")(upper_incl?"<=":"<")(upper)("\n");
	
	Orderings::DepthFirstOrdering::iterator it_candidate = boundaries.lower_bound(lower);
	if( it_candidate == boundaries.end() )
	{
		TRACE("failed, no boundaries at or after lower\n");
		return XLink(); // lower bound rules out
	}
	TRACE("lower_bound(")(lower)(") is ")(*it_candidate)("\n");
	if( *it_candidate==lower && !lower_incl )
	{
		++it_candidate;
	}
	if( it_candidate == boundaries.end() )
	{
		TRACE("failed, no boundaries strictly after lower\n");
		return XLink(); // exclusive lower bound rules out
	}
		
	XLink candidate = *it_candidate;
	
	auto p = dfr.CompareHierarchical(candidate, upper);
	bool ok = upper_incl ? p.first<=0 : p.first<0;

	if( !ok ) 
	{
		TRACE(candidate)(" failed, against upper\n");
		return XLink();
	}
	TRACE("got ")(candidate)(" with ")(p)("\n");
	return candidate;
}


void BoundaryPass::SplitTreeZoneAtXLink( shared_ptr<Patch> *patch_ptr, XLink split_point )
{
	auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*patch_ptr);
	TreeZone *initial_zone = tree_patch->GetZone();
    vector<XLink> parent_terminii, new_terminii;
	list<shared_ptr<Patch>> parent_children, new_children;
	shared_ptr<Patch> *new_patch_ptr = nullptr;

	auto p = dfr.CompareHierarchical(initial_zone->GetBaseXLink(), split_point);
	ASSERT( p.second==DepthFirstRelation::LEFT_IS_ANCESTOR )(initial_zone->GetBaseXLink())(" vs ")(split_point)(" got ")(p); // split point should be in our zone
	
	// Co-walk zone and patch
	size_t terminus_index = 0;
	bool saw_under_split = false;
	bool done_insert = false;
	XLink prev_terminus;
	TreeZonePatch::ForChildren( *patch_ptr, [&](shared_ptr<Patch> &child_patch)    
	{
		ASSERT( terminus_index<initial_zone->GetNumTerminii() ); // length mismatch
		XLink terminus = initial_zone->GetTerminusXLink(terminus_index);
		
		auto p = dfr.CompareHierarchical(split_point, terminus);
		ASSERT( p.second!=DepthFirstRelation::RIGHT_IS_ANCESTOR );
		ASSERT( p.second!=DepthFirstRelation::EQUAL );
		bool is_under_split = (p.second==DepthFirstRelation::LEFT_IS_ANCESTOR);
		bool is_after_split = (!is_under_split && p.first < 0); // term is after split but weak sib, not descended
		bool should_insert = is_after_split && !done_insert;
		
		if( should_insert )
		{
			// We are past the split point and should insert the new patch/zone before terminus
			ASSERT( !prev_terminus || dfr.Compare3Way(prev_terminus, split_point) < 0 );
			parent_terminii.push_back(split_point); 
			MutableTreeZone new_child_zone = db->CreateMutableTreeZone(split_point, new_terminii);
			auto new_child_patch = make_shared<TreeZonePatch>( new_child_zone, move(new_children) );
			parent_children.push_back(new_child_patch);
			new_patch_ptr = &(parent_children.back());
			done_insert = true;
		}

		if( is_under_split )
		{
			// terminus descends from split point -> move to child
			new_terminii.push_back(terminus); 
			new_children.push_back(child_patch);
			saw_under_split = true;
		}			
		else
		{			
			// terminus is weak sibling of split point -> keep in parent
			parent_terminii.push_back(terminus); 
			parent_children.push_back(child_patch);
		}
			
		if( done_insert )
			ASSERT( p.first < 0 );
		else if( is_under_split )
			ASSERT( p.first < 0 && p.second==DepthFirstRelation::LEFT_IS_ANCESTOR );
		else
			ASSERT( p.first > 0 );
		
			
		prev_terminus = terminus;
		terminus_index++;
	} );
	ASSERT( terminus_index==initial_zone->GetNumTerminii() ); // length mismatch

	if( !done_insert )
	{
		ASSERT( !prev_terminus || dfr.Compare3Way(prev_terminus, split_point) < 0 );
		parent_terminii.push_back(split_point); 
		MutableTreeZone new_child_zone = db->CreateMutableTreeZone(split_point, new_terminii);
		auto new_child_patch = make_shared<TreeZonePatch>( new_child_zone, move(new_children) );
		parent_children.push_back(new_child_patch);
		new_patch_ptr = &(parent_children.back());
	}
	
	ASSERT( new_patch_ptr );
	
	XLink px = initial_zone->GetBaseXLink();
	for( XLink x : parent_terminii )
	{
		ASSERT( dfr.Compare3Way(px, x) < 0 );
		px = x;
	}
	
	// Create new parent zone and terminii
	MutableTreeZone parent_zone = db->CreateMutableTreeZone(initial_zone->GetBaseXLink(), parent_terminii);
	auto t = make_shared<TreeZonePatch>( parent_zone, move(parent_children) );	
	
	TRACE("Splitting: ")(patch_ptr)("\ninto: ")(t)("\nand: ")(new_patch_ptr)("\n");
	
	*patch_ptr = t;
	check_queue.push(patch_ptr);
	check_queue.push(new_patch_ptr);
}

















