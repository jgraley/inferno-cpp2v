#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "db/df_relation.hpp"
#include "db/x_tree_database.hpp"
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
    return TreeZone( base, 
                     { base } ); // One element, same as base
}


TreeZone::TreeZone( XLink base_, vector<XLink> terminii_ ) :
    base( base_ ),
    terminii( move(terminii_) )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildX() ); // Cannot be empty
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


vector<XLink> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


XLink TreeZone::GetTerminusXLink(int ti) const
{
	ASSERT( ti >= 0 );
	ASSERT( ti < terminii.size() );
    return terminii.at(ti);
}


void TreeZone::DBCheck( const XTreeDatabase *db ) const
{
	ASSERT( db->HasRow( base ) )(base);
	for( XLink terminus_xlink : terminii )
		ASSERT( db->HasRow( terminus_xlink ) )(terminus_xlink);
}


FreeZone TreeZone::Duplicate( XTreeDatabase *x_tree_db ) const
{
	if( x_tree_db ) // DB is optional
		DBCheck(x_tree_db);

    if( IsEmpty() )
		return FreeZone::CreateEmpty();
    	
    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Terminus>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( x_tree_db, 
                                                            GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    vector<shared_ptr<Terminus>> free_zone_terminii;
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


bool TreeZone::IsOverlap( const XTreeDatabase *db, const TreeZone &l, const TreeZone &r )
{
    DepthFirstRelation df_rel(db);
    const TreeZone *d;
    const TreeZone *a;
    
	TRACES("l=")(l)(" r=")(r);
    
    auto p_base = df_rel.CompareHierarchical(l.base, r.base);
    switch( p_base.second )
    {
        case DepthFirstRelation::SAME:
			TRACES(" identical: overlap\n");
            return true; // identical
            
        case DepthFirstRelation::LEFT_IS_ANCESTOR:
            a = &l;
            d = &r;
            break; // maybe
            
        case DepthFirstRelation::RIGHT_IS_ANCESTOR:
            a = &r;
            d = &l;
            break; // maybe
        
        case DepthFirstRelation::CONTAINER_SIBLINGS:
        case DepthFirstRelation::ITEM_SIBLINGS:
        case DepthFirstRelation::ROOT_SIBLINGS:
			TRACES(" different branches: no overlap\n");
            return false; // weakly removed sibling bases cannot overlap
            
        default:
            ASSERTFAILS();
    }
    
	TRACES(" same branch a=")(a)(" d=")(d);

    // If a has a terminus that is an ancestor (weakly)
    // to d's base, then they do not overlap. Otherwise, they do.
    for( XLink terminus : a->terminii )
    {
		TRACES(" terminus: ")(terminus);
        auto p_term = df_rel.CompareHierarchical(terminus, d->base);
        TRACES(" ")(p_term);
        switch( p_term.second )
        {
            case DepthFirstRelation::SAME:
				TRACES(" touching: no overlap\n");
                return false; // close but no overlap, zone d begins at from a's terminus
                
            case DepthFirstRelation::LEFT_IS_ANCESTOR:
				TRACES(" aa dd: no overlap\n");
                return false; // no overlap, zone d descends from a's terminus
                
            case DepthFirstRelation::RIGHT_IS_ANCESTOR:
				TRACES(" adad: overlap\n");
                return true; // a's terminus descends from base of d, making the zones overlap
            
            case DepthFirstRelation::CONTAINER_SIBLINGS:
            case DepthFirstRelation::ITEM_SIBLINGS:
            case DepthFirstRelation::ROOT_SIBLINGS:
                break; // a's terminus is on a different branch than d
                
            default:
                ASSERTFAILS();
        }                
    }
    
	TRACES(" erm...?: overlap\n");
    return true; // None of a's terminii in path from a's base to zone d
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
