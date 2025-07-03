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


void Patch::AddEmbeddedMarker( PatternLink new_marker )
{
    AddEmbeddedMarkers( { new_marker } );
}


string Patch::GetChildrenTrace() const
{
    list<string> ls;
    for( const shared_ptr<Patch> &child_patch : child_patches )
    {
		string s = SSPrintf("%p->", child_patch.get());
		if( dynamic_cast<TreePatch *>(child_patch.get()) )
			s += "TreePatch";
		else if( dynamic_cast<FreePatch *>(child_patch.get()) )
			s += "FreePatch";
		else
			s += "?";
		ls.push_back(s);
	}
	return Trace(ls);
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
    for( shared_ptr<Patch> &patch : child_patches )
    {
        if( func_in )
            func_in(patch);
        patch->DepthFirstWalkImpl(func_in, func_out);
        if( func_out )
            func_out(patch);
    }
}


// ------------------------- TreePatch --------------------------

TreePatch::TreePatch( const TreeZone &zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(zone_)
{
    ASSERT( zone.GetNumTerminii() == GetNumChildren() );    
}    


TreePatch::TreePatch( const TreeZone &zone_ ) :
    Patch(),
    zone(zone_)
{
    // If zone has terminii, they will be "exposed".
}        


void TreePatch::AddEmbeddedMarkers( list<PatternLink> &&new_markers )
{
    embedded_markers.splice( embedded_markers.end(), move(new_markers) );
}


list<PatternLink> TreePatch::GetEmbeddedMarkers() const
{
    return embedded_markers;
}


void TreePatch::ClearEmbeddedMarkers()
{
    embedded_markers.clear();
}


TreeZone *TreePatch::GetZone() 
{
    return &zone;
}


const TreeZone *TreePatch::GetZone() const
{
    return &zone;
}


TreeZone TreePatch::GetXTreeZone() const
{
	return zone;
}
    

shared_ptr<FreePatch> TreePatch::DuplicateToFree() const
{
    unique_ptr<FreeZone> free_zone = zone.Duplicate();
    list<shared_ptr<Patch>> c = GetChildren();
    auto pop_free_patch = make_shared<FreePatch>( *free_zone, move(c) );
    pop_free_patch->AddEmbeddedMarkers( GetEmbeddedMarkers() );
    return pop_free_patch;
}    


void TreePatch::ForTreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<Patch> &patch)> func )
{
    ForChildren( base, 
	[&](shared_ptr<Patch> &patch)
	{
		if( dynamic_pointer_cast<TreePatch>(patch) )		
			func(patch);
	} );
}
	

void TreePatch::ForTreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<TreePatch> &patch)> func )

{
	ForTreeChildren( base, 
		[&](shared_ptr<Patch> &patch)
		{
			auto p = dynamic_pointer_cast<TreePatch>(patch);
			func(p);
			patch = p; // in case p changed
		} );
}
	

void TreePatch::ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<Patch> &patch)> func_in,
                                           function<void(shared_ptr<Patch> &patch)> func_out )
{
    ForDepthFirstWalk( base, 
		[&](shared_ptr<Patch> &patch)
		{
			if( func_in && dynamic_pointer_cast<TreePatch>(patch) )
				func_in(patch);
		},
		[&](shared_ptr<Patch> &patch)
		{
			if( func_out && dynamic_pointer_cast<TreePatch>(patch) )
				func_out(patch);
		} );
}


void TreePatch::ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<TreePatch> &patch)> func_in,
                                           function<void(shared_ptr<TreePatch> &patch)> func_out )
{
    ForTreeDepthFirstWalk( base, 
		[&](shared_ptr<Patch> &patch)
		{
			if( func_in ) 
			{ 
				auto p = dynamic_pointer_cast<TreePatch>(patch);
				func_in(p);
				patch = p; // in case p changed
			}
		},
		[&](shared_ptr<Patch> &patch)
		{
			if( func_out ) 
			{ 
				auto p = dynamic_pointer_cast<TreePatch>(patch); 
				func_out(p);
				patch = p; // in case p changed
			}
		} );
}


void TreePatch::SetIntent(Intent in)
{
	intent = in;
}


TreePatch::Intent TreePatch::GetIntent() const
{
	return intent;
}


string TreePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
    return "TreePatch( \nzone: "+Trace(zone)+",\nchildren: "+GetChildrenTrace()+" )";
#else
    return "TreePatch( zone: "+Trace(zone)+", "+Trace(GetNumChildren())+" children )";
#endif
}

// ------------------------- FreePatch --------------------------

FreePatch::FreePatch( const FreeZone &zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(zone_)
{
    ASSERT( zone.GetNumTerminii() == GetNumChildren() );    
}

        
FreePatch::FreePatch( const FreeZone &zone_ ) :
       Patch(),
       zone(zone_)
{
    // If zone has terminii, they will be "exposed" and will remain 
    // in the zone returned by Evaluate.
}


void FreePatch::AddEmbeddedMarkers( list<PatternLink> &&new_markers )
{
    // Rule #726 requires us to mark free zones immediately
    for( PatternLink marker : new_markers )
        marker.GetChildAgent()->MarkOriginForEmbedded( zone.GetBaseNode() );    
}


list<PatternLink> FreePatch::GetEmbeddedMarkers() const
{
    return {}; // Rule #726 means there aren't any
}


void FreePatch::ClearEmbeddedMarkers()
{
    // Rule #726 means there aren't any
}


Patch::ChildPatchIterator FreePatch::SpliceOver( ChildPatchIterator it_child, 
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


FreeZone *FreePatch::GetZone()
{
    return &zone;
}


const FreeZone *FreePatch::GetZone() const
{
    return &zone;
}


void FreePatch::ForFreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<Patch> &patch)> func )
{
    ForChildren( base, 
		[&](shared_ptr<Patch> &patch)
		{
			if( dynamic_pointer_cast<FreePatch>(patch) )
				func(patch);
		} );
}


void FreePatch::ForFreeChildren( shared_ptr<Patch> base,
                                     function<void(shared_ptr<FreePatch> &patch)> func )
{
    ForFreeChildren( base, 
		[&](shared_ptr<Patch> &patch)
		{
			auto p = dynamic_pointer_cast<FreePatch>(patch);
			func(p);
			patch = p; // in case p changed
		} );
}


void FreePatch::ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<Patch> &patch)> func_in,
                                           function<void(shared_ptr<Patch> &patch)> func_out )
{
    ForDepthFirstWalk( base, 
		[&](shared_ptr<Patch> &patch)
		{
			if( func_in && dynamic_pointer_cast<FreePatch>(patch) )
				func_in(patch);
		},
		[&](shared_ptr<Patch> &patch)
		{
			if( func_out && dynamic_pointer_cast<FreePatch>(patch) )
				func_out(patch);
		} );
}


void FreePatch::ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                           function<void(shared_ptr<FreePatch> &patch)> func_in,
                                           function<void(shared_ptr<FreePatch> &patch)> func_out )
{
    ForFreeDepthFirstWalk( base, 
		[&](shared_ptr<Patch> &patch)
		{
			if( func_in ) 
			{ 
				auto p = dynamic_pointer_cast<FreePatch>(patch);
				func_in(p);
				patch = p; // in case p changed
			}
		},
		[&](shared_ptr<Patch> &patch)
		{
			if( func_out ) 
			{ 
				auto p = dynamic_pointer_cast<FreePatch>(patch);
				func_out(p);
				patch = p; // in case p changed
			}
		} );
}


string FreePatch::GetTrace() const
{
#ifdef RECURSIVE_TRACE_OPERATOR
    return "FreePatch( \nzone: "+Trace(zone)+",\nchildren: "+GetChildrenTrace()+" )";
#else
    return "FreePatch( zone: "+Trace(zone)+", "+Trace(GetNumChildren())+" children )";
#endif    
}


