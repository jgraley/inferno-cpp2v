#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;

unique_ptr<FreeZone> TreeZone::Duplicate() const
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
    return make_unique<FreeZone>( new_base_x, move(free_zone_terminii) );
}


void TreeZone::Validate(const XTreeDatabase *db) const // TODO maybe move to database?
{
    ASSERT( db->HasRow( GetBaseXLink() ) )(GetBaseXLink());
    
    if( IsEmpty() )
        return; // We've checked enough for empty zones
    // Now we've excluded legit empty zones, checks can be strict
    
    DepthFirstRelation dfr( db );
    XLink prev = XLink();
    XLink base = GetBaseXLink();
    //FTRACE(prev_xlink)("\n");
    for( XLink terminus : GetTerminusXLinks() )
    {
        ASSERT( db->HasRow( terminus ) )(terminus);        
                        
        if( prev )
        {
			auto p = dfr.CompareHierarchical( prev, terminus );
            ASSERT( p.first < 0 ); // strict: no repeated XLinks
            // Terminii should not be in parent/child relationships
			ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR )(prev)(" vs ")(terminus)(" got ")(p); 
		}

        auto p2 = dfr.CompareHierarchical( base, terminus );
		ASSERT( p2.second == DepthFirstRelation::LEFT_IS_ANCESTOR )(base)(" vs ")(terminus)(" got ")(p2); 
            
        prev = terminus;        
    }    
}

// ------------------------- XTreeZone --------------------------

unique_ptr<XTreeZone> XTreeZone::CreateSubtree( XLink base )
{
    return make_unique<XTreeZone>( base, vector<XLink>() );
}


unique_ptr<XTreeZone> XTreeZone::CreateEmpty( XLink base )
{
    ASSERTS( base );
    return make_unique<XTreeZone>( base, vector<XLink>{ base } ); // One element, same as base
}


XTreeZone::XTreeZone( XLink base_, vector<XLink> terminii_ ) :
    base( base_ ),
    terminii( terminii_ )
{
    ASSERT( base ); // XTreeZone is not nullable
    ASSERT( base.GetChildTreePtr() ); // Cannot be empty
}


bool XTreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && SoloElementOf(terminii)==base;    
}


size_t XTreeZone::GetNumTerminii() const
{
    return terminii.size();
}


TreePtr<Node> XTreeZone::GetBaseNode() const
{
    return base.GetChildTreePtr();
}


XLink XTreeZone::GetBaseXLink() const
{
    return base;
}


vector<XLink> XTreeZone::GetTerminusXLinks() const
{
    return terminii;
}


XLink XTreeZone::GetTerminusXLink(size_t index) const
{
	return terminii[index];
}



string XTreeZone::GetTrace() const
{
    string s;
	s = Trace(base);
    if( IsEmpty() )
    {
        s += " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {        
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "XTreeZone(" + s +")";
}

// ------------------------- MutableTreeZone --------------------------

unique_ptr<MutableTreeZone> MutableTreeZone::CreateSubtree( shared_ptr<Mutator> base )
{
    return make_unique<MutableTreeZone>( base, vector<shared_ptr<Mutator>>() );
}


unique_ptr<MutableTreeZone> MutableTreeZone::CreateEmpty( shared_ptr<Mutator> base )
{
    return make_unique<MutableTreeZone>( base, vector<shared_ptr<Mutator>>{ base } ); // One element, same as base
}


MutableTreeZone::MutableTreeZone( shared_ptr<Mutator> base_, 
                                  vector<shared_ptr<Mutator>> &&terminii_ ) :
    base(base_),
    terminii(move(terminii_))
{
}


bool MutableTreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && *SoloElementOf(terminii) == *base;
}


size_t MutableTreeZone::GetNumTerminii() const
{
    return terminii.size();
}


TreePtr<Node> MutableTreeZone::GetBaseNode() const
{
    return base->GetChildTreePtr();
}


XLink MutableTreeZone::GetBaseXLink() const
{
    return base->GetXLink();
}


vector<XLink> MutableTreeZone::GetTerminusXLinks() const
{
	vector<XLink> xlinks;
    for( shared_ptr<Mutator> terminus : terminii )
		xlinks.push_back( terminus->GetXLink() );
	return xlinks;
}


XLink MutableTreeZone::GetTerminusXLink(size_t index) const
{
	return terminii[index]->GetXLink();
}


shared_ptr<Mutator> MutableTreeZone::GetBaseMutator() const
{
    return base;
}


vector<shared_ptr<Mutator>> MutableTreeZone::GetTerminusMutators() const
{
	return terminii;
}


shared_ptr<Mutator> MutableTreeZone::GetTerminusMutator(size_t index) const
{
	return terminii[index];
}


void MutableTreeZone::Exchange( FreeZone *free_zone )
{    
	// Should be true regardless of empty zones
	ASSERT( GetNumTerminii() == free_zone->GetNumTerminii() );

	ASSERT( !free_zone->IsEmpty() ); // Could add support but apparently don't need it rn	       	        
            
	TreePtr<Node> original_tree_zone_base = base->GetChildTreePtr();

    if( IsEmpty() )
    {		
		shared_ptr<Mutator> &tree_terminus = SoloElementOf(terminii);
		TreePtr<Node> free_base = free_zone->GetBaseNode();
		shared_ptr<Mutator> free_terminus = *(free_zone->GetTerminiiBegin()); 
			
		ASSERT( tree_terminus==base );
		auto p = tree_terminus->SplitExchange(free_base, *free_terminus);		
		TreePtr<Node> original_tree_zone_base = p.first;
		tree_terminus = p.second;
    
		free_zone->SetBase( original_tree_zone_base );	
		return;
	}

    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    FreeZone::TerminusIterator free_mut_it = free_zone->GetTerminiiBegin();    
    for( shared_ptr<Mutator> &tree_terminus : terminii )
    {
        ASSERT( free_mut_it != free_zone->GetTerminiiEnd() ); // length mismatch    
                         
	    if( IsEmpty() )
	    {
			// #784 workaround, will NOT alias to other mutators eg in other
			// tree zones, eg ones at these terminii, so you must not use them egain. 
			tree_terminus = tree_terminus->Clone(); 
		}	

		tree_terminus->ExchangeParent(**free_mut_it); // deep
                
        ASSERT( tree_terminus->GetChildTreePtr() );
        free_mut_it++;
    } 
    ASSERT( free_mut_it == free_zone->GetTerminiiEnd() ); // length mismatch  

    // Exchange the base. We want to modify the child side of the base
    // in-place, leaving valid mutators behind. 
    TreePtr<Node> free_base = free_zone->GetBaseNode();
    (void)base->ExchangeChild( free_base );	// deep 
    
    ASSERT( original_tree_zone_base );
	free_zone->SetBase( original_tree_zone_base );	
}


string MutableTreeZone::GetTrace() const
{
    string s;
	s = Trace(base);
    if( IsEmpty() )
    {
        s += " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {       
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "MutableTreeZone(" + s +")";
}
