#include "zone.hpp"

#include "helpers/flatten.hpp"
#include "db/df_relation.hpp"
#include "db/x_tree_database.hpp"

using namespace SR;

// ------------------------- FreeZone --------------------------

FreeZone FreeZone::CreateSubtree( TreePtr<Node> base )
{
    return FreeZone( base, {} );
}


FreeZone FreeZone::CreateEmpty()
{
    return FreeZone( TreePtr<Node>(), // NULL
                     { shared_ptr<Updater>() } ); // One element, NULL
}


FreeZone::FreeZone()
{
}


FreeZone::FreeZone( TreePtr<Node> base_, vector<shared_ptr<Updater>> terminii_ ) :
    base( base_ )
{
	// Fill the map
	for( int ti=0; ti<terminii_.size(); ti++ )
	{
		terminii[ti] = terminii_.at(ti);
		if( base )
			ASSERT( terminii.at(ti) );
	}
	
    // An empty free zone is indicated by a NULL base and exactly one
    // terminus, which should also be NULL.
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !terminii.at(0) );
    }

    // Checks all terminii are distinct
    (void)ToSetSolo(terminii);
}


void FreeZone::AddTerminus(int ti, shared_ptr<Updater> terminus)
{
	// Can't use this to make an empty zone
	ASSERT( base );
	ASSERT( terminus );
	
	terminii[ti] = terminus;
}


TreePtr<Node> FreeZone::GetBaseNode() const
{
    return base;
}


vector<shared_ptr<Updater>> FreeZone::GetTerminusUpdaters() const
{
	vector<shared_ptr<Updater>> v;
	for( int ti=0; ti<terminii.size(); ti++ )
	{
		ASSERT( terminii.count(ti) > 0 );
		v.push_back( terminii.at(ti) );
	}
	
    return v;
}


int FreeZone::GetNumTerminii() const
{
    return terminii.size();
}


shared_ptr<Updater> FreeZone::GetTerminusUpdater(int ti) const
{
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
    return terminii.at(ti);
}


void FreeZone::DropTerminus(int ti)
{
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
	int ne = terminii.erase(ti);
	ASSERT( ne==1 );
}


bool FreeZone::IsEmpty() const
{
    // No base indicates an empty zone
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !terminii.at(0) );
        return true;
    }
    return false;
}


string FreeZone::GetTrace() const
{
    list<string> elts;
    for( auto p : terminii )
        elts.push_back( Trace(p.first)+":"+Trace(p.second) );
    
    string arrow, rhs;
    if( IsEmpty() )
    {
        rhs = " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {
        if( terminii.empty() )
            arrow = " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            arrow = " ⇥ "; // Indicates the zone terminates

        rhs = arrow + Join(elts, ", ");
    }
        
    return "FreeZone(" + Trace(base) + rhs +")";
}


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


XLink TreeZone::GetBaseXLink() const
{
    return base;
}


vector<XLink> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


int TreeZone::GetNumTerminii() const
{
    return terminii.size();
}


XLink TreeZone::GetTerminusXLink(int ti) const
{
	ASSERT( ti >= 0 );
	ASSERT( ti < terminii.size() );
    return terminii.at(ti);
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && OnlyElementOf(terminii)==base;
}


void TreeZone::DBCheck( const XTreeDatabase *db ) const
{
	ASSERT( db->HasRow( base ) )(base);
	for( XLink terminus_xlink : terminii )
		ASSERT( db->HasRow( terminus_xlink ) )(terminus_xlink);
}


bool TreeZone::IsOverlap( const XTreeDatabase *db, const TreeZone &l, const TreeZone &r )
{
    DepthFirstRelation df_rel(db);
    const TreeZone *d;
    const TreeZone *a;
    
    auto p_base = df_rel.CompareHierarchical(l.base, r.base);
    switch( p_base.second )
    {
        case DepthFirstRelation::SAME:
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
            return false; // weakly removed sibling bases cannot overlap
            
        default:
            ASSERTFAILS();
    }
    
    // If a has a terminus that is an ancestor (weakly)
    // to d's base, then they do not overlap. Otherwise, they do.
    for( XLink terminus : a->terminii )
    {
        auto p_term = df_rel.CompareHierarchical(terminus, d->base);
        switch( p_base.second )
        {
            case DepthFirstRelation::SAME:
                return false; // close but no overlap, zone d begins at from a's terminus
                
            case DepthFirstRelation::LEFT_IS_ANCESTOR:
                return false; // no overlap, zone d descends from a's terminus
                
            case DepthFirstRelation::RIGHT_IS_ANCESTOR:
                return true; // a's terminus descends from base of d, making the zones overlap
            
            case DepthFirstRelation::CONTAINER_SIBLINGS:
            case DepthFirstRelation::ITEM_SIBLINGS:
            case DepthFirstRelation::ROOT_SIBLINGS:
                break; // a's terminus is on a different branch than d
                
            default:
                ASSERTFAILS();
        }                
    }
    
    return true; // None of a's terminii in path from a's base to zone d
}


string TreeZone::GetTrace() const
{
    list<string> elts;
    for( const XLink &p : terminii )
        elts.push_back( Trace(p) );

    string rhs;
    if( IsEmpty() )
    {
        rhs = " ↯ "; // Indicates zone is empty due to a terminus at base
                     // (we still give the base, for info)
    }
    else
    {
        string arrow;
        if( terminii.empty() )
            arrow = " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            arrow = " ⇥ "; // Indicates the zone terminates
            
        rhs = arrow + Join(elts, ", ");
    }
    
    return "TreeZone(" + Trace(base) + rhs +")";
}
