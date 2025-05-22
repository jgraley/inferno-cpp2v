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
    list<string> ls;
    for( const shared_ptr<Patch> &child_patch : child_patches )
    {
		string s = SSPrintf("%p->", child_patch.get());
		if( dynamic_cast<TreeZonePatch *>(child_patch.get()) )
			s += "TreeZonePatch";
		else if( dynamic_cast<FreeZonePatch *>(child_patch.get()) )
			s += "FreeZonePatch";
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


// ------------------------- TreeZonePatch --------------------------

TreeZonePatch::TreeZonePatch( const XTreeZone &zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(make_unique<XTreeZone>(zone_))
{
    ASSERT( zone->GetNumTerminii() == GetNumChildren() );    
}    
        

TreeZonePatch::TreeZonePatch( const MutableTreeZone &zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(make_unique<MutableTreeZone>(zone_))
{
    ASSERT( zone->GetNumTerminii() == GetNumChildren() );    
}    
        

TreeZonePatch::TreeZonePatch( const XTreeZone &zone_ ) :
    Patch(),
    zone(make_unique<XTreeZone>(zone_))
{
    // If zone has terminii, they will be "exposed".
}        

TreeZonePatch::TreeZonePatch( const MutableTreeZone &zone_ ) :
    Patch(),
    zone(make_unique<MutableTreeZone>(zone_))
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
    auto pop_free_patch = make_shared<FreeZonePatch>( *free_zone, move(c) );
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

FreeZonePatch::FreeZonePatch( const FreeZone &zone_, 
                              list<shared_ptr<Patch>> &&child_patches ) :
    Patch( move(child_patches) ),
    zone(make_unique<FreeZone>(zone_))
{
    ASSERT( zone->GetNumTerminii() == GetNumChildren() );    
}

        
FreeZonePatch::FreeZonePatch( const FreeZone &zone_ ) :
       Patch(),
       zone(make_unique<FreeZone>(zone_))
{
    // If zone has terminii, they will be "exposed" and will remain 
    // in the zone returned by Evaluate.
}


void FreeZonePatch::AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers )
{
    // Rule #726 requires us to mark free zones immediately
    for( RequiresSubordinateSCREngine *ea : new_markers )
        zone->MarkOriginForEmbedded(ea);    
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

// ------------------------- Utils ---------------------------

pair<DBCommon::TreeOrdinal, MutableTreeZone> SR::FreeZoneIntoExtraTree( XTreeDatabase *db, FreeZone free_zone, const MutableTreeZone &reference_tree_zone )
{
	// Reference tree zone is used to determine the types of the plugs

	vector<XLink> terminii;
    FreeZone::TerminusIterator terminus_it = free_zone.GetTerminiiBegin();
    for( size_t i=0; i<reference_tree_zone.GetNumTerminii(); i++ )
	{
		// Plug the terminii of the "from" scaffold with yet more scaffolding so we get a subtree for the extra tree.
		// This is a requirement for placing a zone (generally including terminii) into its own extra tree. Alternatively
		// we could allow NULL TreePtrs/placeholders to exist in tree and define semantics for them.
		TreePtr<Node> term_child_node = reference_tree_zone.GetTerminusMutator(i).GetChildTreePtr();
		ASSERT(term_child_node);
		FreeZone plug = FreeZone::CreateScaffoldToSpec(term_child_node, 0); // finally no terminii!!!
		Mutator resultant_mutator;
        terminus_it = free_zone.MergeTerminus( terminus_it, make_unique<FreeZone>(plug), &resultant_mutator );
        terminii.push_back(resultant_mutator.GetXLink());
	}	
	TRACE("Free zone after populating terminii: ")(free_zone)("\n");

    // Add a new extra tree containing the plugged "from" scaffold
    DBCommon::TreeOrdinal extra_tree_ordinal = db->AllocateExtraTree();        
	TRACE("Allocated extra tree %u\n", extra_tree_ordinal);
    MutableTreeZone zone_in_extra_tree = db->BuildTree( extra_tree_ordinal, free_zone );
	TRACE("Zone in extra tree: ")(zone_in_extra_tree)("\n");

	// ------------------------- Get unplugged zone for our scaffold ---------------------------
	// We require a TZ based on the "from" scaffold that resembles main_tree_zone_from, with real
	// TZ terminii, even though we plugged the FZ terminii making it a subtree.
	XLink root_xlink = db->GetRootXLink(extra_tree_ordinal);
	TRACE("Extra tree root: ")(root_xlink)("\n");
	MutableTreeZone tree_zone_in_extra = db->CreateMutableTreeZone( root_xlink, terminii );	
	TRACE("Original zone in TZ with its terminii: ")(tree_zone_in_extra)("\n");
	
	return make_pair(extra_tree_ordinal, tree_zone_in_extra);

}



