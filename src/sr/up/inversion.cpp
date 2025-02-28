#include "inversion.hpp"

#include "patches.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneInverter::TreeZoneInverter( XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneInverter::Run(TreeZone target_base, shared_ptr<Patch> *source_layout_ptr)
{
	LocatedZoneExpression base_lze( target_base.GetBaseXLink(), source_layout_ptr );
	WalkFreeZoneExpr( base_lze );
}


void TreeZoneInverter::WalkFreeZoneExpr( LocatedZoneExpression lze )
{
	// Really just a search for FreeZonePatch that fills in the target base XLink from the 
	// enclosing thing (if it's root or a tree zone). 
	// Inversion strategy: this XLink is available for every free zone because we did free zone
	// merging (if parent was a free zone, we'd have no XLink)
	if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(*lze.second) )
	{
		// Free zone: recurse and then invert locally
		free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)	
		{
			// We don't know the base if we're coming from a free zone
			ASSERT( dynamic_pointer_cast<TreeZonePatch>(child_patch) )
			      ("FZ under another FZ (probably), cannot determine target XLink");
			LocatedZoneExpression child_lze( XLink(), &child_patch );
			WalkFreeZoneExpr( child_lze );
		} );
	
		// Invert the free zone while unwinding
		Invert(lze); 
	}
	else if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*lze.second) )
	{
		// Recurse, co-looping over the children/terminii so we can fill in target bases
		vector<XLink> terminii = tree_patch->GetZone().GetTerminusXLinks();
		FreeZonePatch::ChildExpressionIterator it_child = tree_patch->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
			
			LocatedZoneExpression child_lze( terminus_xlink, &*it_child );
			WalkFreeZoneExpr( child_lze );
						
			++it_child;
		}
		ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
	}
	else
		ASSERTFAIL();
}


void TreeZoneInverter::Invert( LocatedZoneExpression lze )
{
	// Checks
	ASSERT( lze.first && lze.second && *lze.second);
	XLink base_xlink = lze.first;
	ASSERT( base_xlink )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
	auto free_patch = dynamic_pointer_cast<FreeZonePatch>( *lze.second );
	ASSERT( free_patch )("Got LZE not a free zone: ")(lze); // WalkFreeZoneExpr() gave us wrong kind of patch
			
	// Collect base xlinks for child zones (which must be tree zones)
	vector<XLink> terminii_xlinks;
	free_patch->ForChildren([&](shared_ptr<Patch> &child_patch)	
	{
		auto child_tree_patch = dynamic_pointer_cast<TreeZonePatch>( child_patch );		
		// Inversion strategy: we're based on a free zone and FZ merging should 
		// have ensured we'll see only tree zones as children. Each base is a terminus 
		// for the new tree zone.
		ASSERT( child_tree_patch ); 
		
		terminii_xlinks.push_back( child_tree_patch->GetZone().GetBaseXLink() );
	} );
		 
	// Make the inverted TZ
	TreeZone inverted_tree_zone = TreeZone( base_xlink, terminii_xlinks );	
	
	// Modify the expression to include inverted TZ as target
	*lze.second = make_shared<TargettedPatch>( inverted_tree_zone,
	                                           make_shared<FreeZone>( free_patch->GetZone() ),
	                                           free_patch->MoveChildExpressions() );   		
}
