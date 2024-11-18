#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;

// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateSubtree( XLink base )
{
    return TreeZone( base, {} );
}


TreeZone TreeZone::CreateEmpty( XLink base )
{
    ASSERTS( base );
    return TreeZone( base, { base } ); // One element, same as base
}


TreeZone::TreeZone( XLink base_, vector<XLink> terminii_ ) :
    base( base_ ),
    terminii( terminii_ )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildTreePtr() ); // Cannot be empty
}


TreeZone::TreeZone( const TreeZone &other ) :
    base( other.base ),
    terminii( other.terminii )
{
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && OnlyElementOf(terminii)==base;
}


int TreeZone::GetNumTerminii() const
{
    return terminii.size();
}


TreePtr<Node> TreeZone::GetBaseNode() const
{
	return base.GetChildTreePtr();
}


XLink TreeZone::GetBaseXLink() const
{
    return base;
}


XLink &TreeZone::GetBaseXLink()
{
    return base;
}


vector<XLink> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


FreeZone TreeZone::Duplicate() const
{
    if( IsEmpty() )
		return FreeZone::CreateEmpty();
    	
    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<FreeTerminus>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    list<shared_ptr<FreeTerminus>> free_zone_terminii;
    for( XLink terminus_upd : GetTerminusXLinks() )
    {
		ASSERTS( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    return FreeZone( new_base_x, move(free_zone_terminii) );
}


TreeZone::TerminusIterator TreeZone::GetTerminiiBegin() const
{
	return terminii.begin();
}


TreeZone::TerminusIterator TreeZone::GetTerminiiEnd() const
{
	return terminii.end();
}


void TreeZone::ReplaceWithFreeZone( FreeZone &&free_zone )
{	
    // Acts on all copies of the xlink, due to indirection, possibly including root as held by db 
    base.SetXPtr( free_zone.GetBaseNode() ); 
    
	// Do a co-walk and populate one at a time. Update our terminii as we go. Cannot use 
	// SetXPtr() because we need to change the PARENT node, so we need a whole new XLink. Do this
	// under a validity check that all our XLins are inside the new tree.
	// This is needed because UpdateMainTree() will pass us to InsertMainTree()
	FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();	
    for( XLink &tree_terminus_xlink : terminii )
	{
		ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch	
		shared_ptr<FreeTerminus> free_terminus = *it_t;
		
		TreePtr<Node> boundary_node = tree_terminus_xlink.GetChildTreePtr(); // outside the zone		
		ASSERT( !dynamic_cast<ContainerInterface *>(boundary_node.get()) ); // requirement for GetTreePtrInterface()
		it_t = free_zone.PopulateTerminus( it_t, boundary_node );		
		
		TreePtr<Node> free_parent_node = free_terminus->GetParentNode();
		const TreePtrInterface *free_tpi = free_terminus->GetTreePtrInterface(); // must be taken after the populate		
		ASSERT( *free_tpi == boundary_node );
		tree_terminus_xlink = XLink(free_parent_node, free_tpi);
	} 
	ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch	
}


string TreeZone::GetTrace() const
{
    string rhs;
    if( IsEmpty() )
    {
        rhs = " ↯ "; // Indicates zone is empty due to a terminus at base
                     // (we still give the base, for info)
    }
    else
    {
        if( terminii.empty() )
            rhs = " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            rhs = " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "TreeZone(" + Trace(base) + rhs +")";
}


void TreeZone::DBCheck(const XTreeDatabase *db) const // TODO maybe move to database?
{
	ASSERT( db->HasRow( base ) )(base);
	
	if( IsEmpty() )
		return; // We've checked enough for empty zones
	// Now we've excluded legit empty zones, checks can be strict
	
	DepthFirstRelation dfr( db );
	XLink prev_xlink = base;
	for( XLink terminus_xlink : terminii )
	{
		ASSERT( db->HasRow( terminus_xlink ) )(terminus_xlink);
		
		ASSERT( dfr.Compare3Way( prev_xlink, terminus_xlink ) < 0 ); // strict: no repeated XLinks
			
		prev_xlink = terminus_xlink;
	}	
}
