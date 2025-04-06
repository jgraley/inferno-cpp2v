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


FreeZone TreeZone::MakeFreeZone(const XTreeDatabase *db) const
{    
    if( IsEmpty() )
        return FreeZone::CreateEmpty();
        
    FreeZone free_zone( base.GetChildTreePtr(), {} );
    for( XLink terminus : terminii )
        free_zone.AddTerminus( db->GetMutator( terminus ) );
    
    return free_zone;
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

MutableTreeZone::MutableTreeZone( TreeZone tz, 
                                  shared_ptr<Mutator> &&base_mutator_, 
                                  vector<shared_ptr<Mutator>> &&terminii_mutators_ ) :
    TreeZone(tz),
    base_mutator(move(base_mutator_)),
    terminii_mutators(move(terminii_mutators_))
{
}



void MutableTreeZone::Exchange( FreeZone &free_zone )
{    
	ASSERT( !free_zone.IsEmpty() ); // Could add support but apparently don't need it rn
	        
    if( IsEmpty() )
    {
        free_zone = FreeZone::CreateEmpty(); // Dodgy but it is reached
        return;
	}
            
    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    FreeZone::TerminusIterator free_mut_it = free_zone.GetTerminiiBegin();    
    vector<XLink>::iterator tree_xlink_it = terminii.begin();
    for( shared_ptr<Mutator> tree_terminus : terminii_mutators )
    {
        ASSERT( free_mut_it != free_zone.GetTerminiiEnd() ); // length mismatch    
        ASSERT( tree_xlink_it != terminii.end() ); // length mismatch    
                                	
		tree_terminus->ExchangeParent(**free_mut_it); // deep

        *tree_xlink_it = tree_terminus->GetXLink();   
                
        free_mut_it++;
        tree_xlink_it++;
    } 
    ASSERT( free_mut_it == free_zone.GetTerminiiEnd() ); // length mismatch  
    ASSERT( tree_xlink_it == terminii.end() ); // length mismatch    

    // Exchange the base. We want to modify the child side of the base
    // in-place, leaving valid mutators behind. 
    TreePtr<Node> free_base = free_zone.GetBaseNode();
    TreePtr<Node> old_base = base_mutator->ExchangeChild( free_base );	// deep
    if( !base_mutator->IsAtRoot() )
		base = base_mutator->GetXLink();   
    
	free_zone.SetBase( old_base );	
}



