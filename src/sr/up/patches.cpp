#include "patches.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

#define RECURSIVE_TRACE_OPERATOR

// ------------------------- Patch --------------------------

Patch::Patch( list<shared_ptr<Patch>> &&child_patches_ ) :
	child_patches(move(child_patches_))
{
}	


Patch::Patch() 
{
}	


int Patch::GetNumChildExpressions() const
{
	return child_patches.size();
}


Patch::ChildExpressionIterator Patch::GetChildrenBegin()
{
	return child_patches.begin();
}


Patch::ChildExpressionIterator Patch::GetChildrenEnd()
{
	return child_patches.end();
}


list<shared_ptr<Patch>> &Patch::GetChildExpressions() 
{
	return child_patches;
}


const list<shared_ptr<Patch>> &Patch::GetChildExpressions() const
{
	return child_patches;
}


list<shared_ptr<Patch>> &&Patch::MoveChildExpressions()
{
	return move(child_patches);
}


string Patch::GetChildExpressionsTrace() const
{
	return Trace(child_patches);
}


void Patch::ForChildren(function<void(shared_ptr<Patch> &patch)> func) try
{
	for( shared_ptr<Patch> &child_patch : child_patches )
		func(child_patch);
}
catch( BreakException )
{
}		       


void Patch::ForDepthFirstWalk( shared_ptr<Patch> &base,
											function<void(shared_ptr<Patch> &patch)> func_in,
	                                        function<void(shared_ptr<Patch> &patch)> func_out ) try
{
	if( func_in )
		func_in(base);
	base->DepthFirstWalkImpl(func_in, func_out);
	if( func_out )
		func_out(base);
}
catch( BreakException )
{
}		       


void Patch::DepthFirstWalkImpl( function<void(shared_ptr<Patch> &patch)> func_in,
			                             function<void(shared_ptr<Patch> &patch)> func_out )
{
	for( shared_ptr<Patch> &patch : child_patches )
	{
		if( func_in )
			func_in(patch);
		patch->DepthFirstWalkImpl(func_in, func_out);
		if( func_out )
			func_out(patch);
	}
}

// ------------------------- ZonePatch --------------------------

ZonePatch::ZonePatch( list<shared_ptr<Patch>> &&child_patches_ ) :
	Patch(move(child_patches_))
{
}	


ZonePatch::ZonePatch() 
{
}	


void ZonePatch::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
	AddEmbeddedMarkers( { new_marker } );
}

// ------------------------- TreeZonePatch --------------------------

TreeZonePatch::TreeZonePatch( TreeZone zone_, 
                                                    list<shared_ptr<Patch>> &&child_patches ) :
	ZonePatch( move(child_patches) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}	
		

TreeZonePatch::TreeZonePatch( TreeZone zone_ ) :
	ZonePatch(),
	zone(zone_)
{
	// If zone has terminii, they will be "exposed".
}


void TreeZonePatch::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	embedded_markers.splice( embedded_markers.end(), move(new_markers) );
}


list<RequiresSubordinateSCREngine *> TreeZonePatch::GetEmbeddedMarkers() const
{
	return embedded_markers;
}


void TreeZonePatch::ClearEmbeddedMarkers()
{
	embedded_markers.clear();
}


TreeZone &TreeZonePatch::GetZone() 
{
    return zone;
}


const TreeZone &TreeZonePatch::GetZone() const
{
    return zone;
}


shared_ptr<Patch> TreeZonePatch::DuplicateToFree() const
{
	FreeZone free_zone = zone.Duplicate();
	list<shared_ptr<Patch>> c = GetChildExpressions();
	auto pop_free_patch = make_shared<FreeZonePatch>( free_zone, move(c) );
	pop_free_patch->AddEmbeddedMarkers( GetEmbeddedMarkers() );
	return pop_free_patch;
}	


string TreeZonePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "TreeZonePatch( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "TreeZonePatch( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif
}

// ------------------------- FreeZonePatch --------------------------

FreeZonePatch::FreeZonePatch( FreeZone zone_, 
                                                    list<shared_ptr<Patch>> &&child_patches ) :
	ZonePatch( move(child_patches) ),
	zone(zone_)
{
	ASSERT( zone.GetNumTerminii() == GetNumChildExpressions() );	
}

		
FreeZonePatch::FreeZonePatch( FreeZone zone_ ) :
   	ZonePatch(),
   	zone(zone_)
{
	// If zone has terminii, they will be "exposed" and will remain 
	// in the zone returned by Evaluate.
}


void FreeZonePatch::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
	// Rule #726 requires us to mark free zones immediately
	for( RequiresSubordinateSCREngine *ea : new_markers )
		zone.MarkBaseForEmbedded(ea);	
}


list<RequiresSubordinateSCREngine *> FreeZonePatch::GetEmbeddedMarkers() const
{
	return {}; // Rule #726 means there aren't any
}


void FreeZonePatch::ClearEmbeddedMarkers()
{
	// Rule #726 means there aren't any
}


ZonePatch::ChildExpressionIterator FreeZonePatch::SpliceOver( ChildExpressionIterator it_child, 
                                                              list<shared_ptr<Patch>> &&child_patches )
{
	// it_child updated to the next child after the one we erased, or end()
	it_child = GetChildExpressions().erase( it_child );		
	
	// Insert the child before it_child, i.e. where we just
	// erase()d from. I assume it_child now points after the inserted 
	// child, i.e. at the same element it did after the erase()
	GetChildExpressions().splice( it_child, move(child_patches) );	
	
	return it_child;
}                                               


FreeZone &FreeZonePatch::GetZone()
{
    return zone;
}


const FreeZone &FreeZonePatch::GetZone() const
{
    return zone;
}


string FreeZonePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
	return "FreeZonePatch( \nzone: "+Trace(zone)+",\nchildren: "+GetChildExpressionsTrace()+" )";
#else
	return "FreeZonePatch( zone: "+Trace(zone)+", "+Trace(GetNumChildExpressions())+" children )";
#endif	
}

// ------------------------- TargettedPatch --------------------------

TargettedPatch::TargettedPatch( TreeZone target_tree_zone_, 
                                  shared_ptr<Zone> source_zone_,
                                  list<shared_ptr<Patch>> &&child_patches ) :
	Patch( move(child_patches) ),
	target_tree_zone(target_tree_zone_),
	source_zone(source_zone_)
{
	ASSERT( target_tree_zone.GetNumTerminii() == source_zone->GetNumTerminii() );
	ASSERT( target_tree_zone.GetNumTerminii() == GetNumChildExpressions() );	
}


const TreeZone &TargettedPatch::GetTargetTreeZone() const
{
	return target_tree_zone;
}


shared_ptr<Zone> TargettedPatch::GetSourceZone() const
{
	return source_zone;
}

