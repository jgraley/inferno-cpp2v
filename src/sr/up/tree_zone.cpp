#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "db/df_relation.hpp"
#include "db/x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;

// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateSubtree( const XTreeDatabase *db, XLink base )
{
    return TreeZone( db, base, {} );
}


TreeZone TreeZone::CreateEmpty( const XTreeDatabase *db, XLink base )
{
    ASSERTS( base );
    return TreeZone( db, base, { base } ); // One element, same as base
}


TreeZone::TreeZone( const XTreeDatabase *db_, XLink base_, vector<XLink> terminii_ ) :
	db( db_ ),
	df_rel( db ),
    base( base_ ),
    terminii( terminii_.begin(), terminii_.end(), df_rel )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildX() ); // Cannot be empty
	DBCheck();
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
	return base.GetChildX();
}


XLink TreeZone::GetBaseXLink() const
{
    return base;
}


set<XLink, DepthFirstRelation> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


FreeZone TreeZone::Duplicate() const
{
	DBCheck();

    if( IsEmpty() )
		return FreeZone::CreateEmpty();
    	
    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Terminus>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( db, 
                                                            GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    list<shared_ptr<Terminus>> free_zone_terminii;
    for( XLink terminus_upd : GetTerminusXLinks() )
    {
		ASSERTS( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    return FreeZone( new_base_x, free_zone_terminii );
}


void TreeZone::Update( XTreeDatabase *x_tree_db, const FreeZone &free_zone ) const
{
	ASSERT( GetNumTerminii() == free_zone.GetNumTerminii() );	
	ASSERT( GetNumTerminii() == 0 ); // TODO under #723
    
    // Update database 
    x_tree_db->Delete( GetBaseXLink() );    
    
    // Patch the tree
    GetBaseXLink().SetXPtr( free_zone.GetBaseNode() );
    
    // Update database 
    x_tree_db->Insert( GetBaseXLink() );   	
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


void TreeZone::DBCheck() const
{
	if( db ) // db is optional: builders leave it NULL
	{
		ASSERT( db->HasRow( base ) )(base);
		for( XLink terminus_xlink : terminii )
			ASSERT( db->HasRow( terminus_xlink ) )(terminus_xlink);
	}
}
