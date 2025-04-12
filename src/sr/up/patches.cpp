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


size_t Patch::GetNumChildren() const
{
    return child_patches.size();
}


Patch::ChildPatchIterator Patch::GetChildrenBegin()
{
    return child_patches.begin();
}


Patch::ChildPatchIterator Patch::GetChildrenEnd()
{
    return child_patches.end();
}


list<shared_ptr<Patch>> &Patch::GetChildren() 
{
    return child_patches;
}


const list<shared_ptr<Patch>> &Patch::GetChildren() const
{
    return child_patches;
}


list<shared_ptr<Patch>> &&Patch::MoveChildren()
{
    return move(child_patches);
}


void Patch::AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker )
{
    AddEmbeddedMarkers( { new_marker } );
}


string Patch::GetChildrenTrace() const
{
    return Trace(child_patches);
}


void Patch::ForChildren( shared_ptr<Patch> base,
                         function<void(shared_ptr<Patch> &patch)> func ) try
{
    for( shared_ptr<Patch> &child_patch : base->child_patches )
        func(child_patch);
}
catch( const BreakException & )
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
catch( const BreakException & )
{
}               


void Patch::DepthFirstWalkImpl( function<void(shared_ptr<Patch> &patch)> func_in,
                                function<void(shared_ptr<Patch> &patch)> func_out )
{
	INDENT(".");
    for( shared_ptr<Patch> &patch : child_patches )
    {
        if( func_in )
            func_in(patch);
        patch->DepthFirstWalkImpl(func_in, func_out);
        if( func_out )
            func_out(patch);
    }
}


// ------------------------- TreeZonePatch --------------------------

TreeZonePatch::TreeZonePatch( unique_ptr<TreeZone> zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(move(zone_))
{
    ASSERT( zone->GetNumTerminii() == GetNumChildren() );    
}    
        

TreeZonePatch::TreeZonePatch( unique_ptr<TreeZone> zone_ ) :
    Patch(),
    zone(move(zone_))
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


TreeZone *TreeZonePatch::GetZone() 
{
    return zone.get();
}


const TreeZone *TreeZonePatch::GetZone() const
{
    return zone.get();
}


void TreeZonePatch::SetZone( unique_ptr<TreeZone> &&new_zone )
{
	zone = move(new_zone);
}


shared_ptr<FreeZonePatch> TreeZonePatch::DuplicateToFree() const
{
    unique_ptr<FreeZone> free_zone = zone->Duplicate();
    list<shared_ptr<Patch>> c = GetChildren();
    auto pop_free_patch = make_shared<FreeZonePatch>( move(free_zone), move(c) );
    pop_free_patch->AddEmbeddedMarkers( GetEmbeddedMarkers() );
    return pop_free_patch;
}    


void TreeZonePatch::ForTreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<Patch> &patch)> func )
{
    ForChildren( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( dynamic_pointer_cast<TreeZonePatch>(patch) )		
			func(patch);
	} );
}
	

void TreeZonePatch::ForTreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<TreeZonePatch> &patch)> func )

{
	ForTreeChildren( base, 
	[&](shared_ptr<Patch> &patch)
	{
		auto p = dynamic_pointer_cast<TreeZonePatch>(patch);
		func(p);
		patch = p; // in case p changed
	} );
}
	

void TreeZonePatch::ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<Patch> &patch)> func_in,
                                           function<void(shared_ptr<Patch> &patch)> func_out )
{
    ForDepthFirstWalk( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( func_in && dynamic_pointer_cast<TreeZonePatch>(patch) )
			func_in(patch);
	},
	[&](shared_ptr<Patch> &patch)
	{
		if( func_out && dynamic_pointer_cast<TreeZonePatch>(patch) )
			func_out(patch);
	} );
}


void TreeZonePatch::ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<TreeZonePatch> &patch)> func_in,
                                           function<void(shared_ptr<TreeZonePatch> &patch)> func_out )
{
    ForTreeDepthFirstWalk( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( func_in ) 
		{ 
			auto p = dynamic_pointer_cast<TreeZonePatch>(patch);
			func_in(p);
			patch = p; // in case p changed
		}
	},
	[&](shared_ptr<Patch> &patch)
	{
		if( func_out ) 
		{ 
			auto p = dynamic_pointer_cast<TreeZonePatch>(patch); 
			func_out(p);
			patch = p; // in case p changed
		}
	} );
}


string TreeZonePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
    return "TreeZonePatch( \nzone: "+Trace(*zone)+",\nchildren: "+GetChildrenTrace()+" )";
#else
    return "TreeZonePatch( zone: "+Trace(*zone)+", "+Trace(GetNumChildren())+" children )";
#endif
}

// ------------------------- FreeZonePatch --------------------------

FreeZonePatch::FreeZonePatch( unique_ptr<FreeZone> zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(move(zone_))
{
    ASSERT( zone->GetNumTerminii() == GetNumChildren() );    
}

        
FreeZonePatch::FreeZonePatch( unique_ptr<FreeZone> zone_ ) :
       Patch(),
       zone(move(zone_))
{
    // If zone has terminii, they will be "exposed" and will remain 
    // in the zone returned by Evaluate.
}


void FreeZonePatch::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
    // Rule #726 requires us to mark free zones immediately
    for( RequiresSubordinateSCREngine *ea : new_markers )
        zone->MarkBaseForEmbedded(ea);    
}


list<RequiresSubordinateSCREngine *> FreeZonePatch::GetEmbeddedMarkers() const
{
    return {}; // Rule #726 means there aren't any
}


void FreeZonePatch::ClearEmbeddedMarkers()
{
    // Rule #726 means there aren't any
}


Patch::ChildPatchIterator FreeZonePatch::SpliceOver( ChildPatchIterator it_child, 
                                                     list<shared_ptr<Patch>> &&child_patches )
{
    // it_child updated to the next child after the one we erased, or end()
    it_child = GetChildren().erase( it_child );        
    
    // Insert the child before it_child, i.e. where we just
    // erase()d from. I assume it_child now points after the inserted 
    // child, i.e. at the same element it did after the erase()
    GetChildren().splice( it_child, move(child_patches) );    
    
    return it_child;
}                                               


FreeZone *FreeZonePatch::GetZone()
{
    return zone.get();
}


const FreeZone *FreeZonePatch::GetZone() const
{
    return zone.get();
}


void FreeZonePatch::ForFreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<Patch> &patch)> func )
{
    ForChildren( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( dynamic_pointer_cast<FreeZonePatch>(patch) )
			func(patch);
	} );
}


void FreeZonePatch::ForFreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<FreeZonePatch> &patch)> func )
{
    ForFreeChildren( base, 
	[&](shared_ptr<Patch> &patch)
	{
		auto p = dynamic_pointer_cast<FreeZonePatch>(patch);
		func(p);
		patch = p; // in case p changed
	} );
}


void FreeZonePatch::ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<Patch> &patch)> func_in,
                                           function<void(shared_ptr<Patch> &patch)> func_out )
{
    ForDepthFirstWalk( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( func_in && dynamic_pointer_cast<FreeZonePatch>(patch) )
			func_in(patch);
	},
	[&](shared_ptr<Patch> &patch)
	{
		if( func_out && dynamic_pointer_cast<FreeZonePatch>(patch) )
			func_out(patch);
	} );
}


void FreeZonePatch::ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<FreeZonePatch> &patch)> func_in,
                                           function<void(shared_ptr<FreeZonePatch> &patch)> func_out )
{
    ForFreeDepthFirstWalk( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( func_in ) 
		{ 
			auto p = dynamic_pointer_cast<FreeZonePatch>(patch);
			func_in(p);
			patch = p; // in case p changed
		}
	},
	[&](shared_ptr<Patch> &patch)
	{
		if( func_out ) 
		{ 
			auto p = dynamic_pointer_cast<FreeZonePatch>(patch);
			func_out(p);
			patch = p; // in case p changed
		}
	} );
}


string FreeZonePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
    return "FreeZonePatch( \nzone: "+Trace(*zone)+",\nchildren: "+GetChildrenTrace()+" )";
#else
    return "FreeZonePatch( zone: "+Trace(*zone)+", "+Trace(GetNumChildren())+" children )";
#endif    
}
