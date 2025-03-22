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
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Mutator>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    list<shared_ptr<Mutator>> free_zone_terminii;
    for( XLink terminus_upd : GetTerminusXLinks() )
    {
		ASSERTS( duplicator_terminus_map[terminus_upd].mutator );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].mutator );
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


string TreeZone::GetTrace() const
{
    string s;
    if( IsEmpty() )
    {
        s = " ↯ "; // Indicates zone is empty due to a terminus at base
                   // (we still give the base, for info)
    }
    else
    {
		s = Trace(base);
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "TreeZone(" + s +")";
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

// ------------------------- MutableTreeZone --------------------------

MutableTreeZone::MutableTreeZone( TreeZone tz, unique_ptr<Mutator> &&base_mutator_ ) :
	TreeZone(tz),
	base_mutator(move(base_mutator_))
{
}

void MutableTreeZone::Patch( FreeZone &&free_zone )
{	
	//FreeZone displaced_free_zone( base, {} );
	bool empty_free_zone = free_zone.IsEmpty();
    
	// Do a co-walk and populate one at a time. Update our terminii as we go. Cannot use 
	// TPI because we need to change the PARENT node, so we need a whole new XLink. Do this
	// under a validity check that all our XLinks are inside the new tree.
	// This is needed because MainTreeReplace() will pass us to MainTreeInsert()
	FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();	
    for( XLink &tree_terminus_xlink : terminii )
	{
		ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch	
		
		// Make the FZ terminus match the TZ terminus
		shared_ptr<Mutator> mutator = *it_t;		
		TreePtr<Node> boundary_node = tree_terminus_xlink.GetChildTreePtr(); // outside the zone		
		ASSERT( !dynamic_cast<ContainerInterface *>(boundary_node.get()) ); // requirement for GetTreePtrInterface()
		it_t = free_zone.PopulateTerminus( it_t, boundary_node ); // Note: ensures free zone not empty	
		
		// Update the tree zone terminus
		if( !empty_free_zone )
		{
			TreePtr<Node> free_parent_node = mutator->GetParentNode();
			ASSERT( free_parent_node ); // NOT compulsory in Mutators
			const TreePtrInterface *free_tpi = mutator->GetTreePtrInterface(); // must be taken after the populate		
			ASSERT( *free_tpi == boundary_node );
			tree_terminus_xlink = XLink(free_parent_node, free_tpi);
		}
	} 
	ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch	

	// Do this after, since free zone is now not empty 
	base_mutator->Mutate( free_zone.GetBaseNode() );

	if( empty_free_zone )
	{
		// Become an empty tree zone
		terminii = { base };
	}
}



