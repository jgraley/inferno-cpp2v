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
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), Mutator() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    list<Mutator> free_zone_terminii;
    for( XLink terminus_upd : GetTerminusXLinks() )
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].mutator );   

    // Create a new zone for the result.
    return make_unique<FreeZone>( new_base_x, move(free_zone_terminii) );
}


void TreeZone::Validate(const XTreeDatabase *db) const // TODO maybe move to database?
{       
    XLink base = GetBaseXLink();
    ASSERT( base )(base);
    ASSERT( base.GetChildTreePtr() )(base);
    ASSERT( db->HasRow( base ) )(base);      

    if( IsEmpty() )
        return; // We've checked enough for empty zones
    // Now we've excluded legit empty zones, checks can be strict
    
    DepthFirstRelation dfr( db );
    XLink prev = XLink();
    
    //FTRACE(prev_xlink)("\n");
    for( XLink terminus : GetTerminusXLinks() )
    {
		ASSERT( terminus )(terminus);
		ASSERT( terminus.GetChildTreePtr() )(terminus);
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


unique_ptr<XTreeZone> XTreeZone::CreateFromScaffold( XLink scaffold_xlink )
{
	TreePtr<Node> scaffold = scaffold_xlink.GetChildTreePtr();
	ScaffoldBase *sbp = dynamic_cast<ScaffoldBase *>(scaffold.get());

	vector<XLink> terminii;
	for( TreePtr<Node> &tpp : sbp->child_ptrs )
		terminii.push_back( XLink( scaffold, &tpp ) );
	
	return make_unique<XTreeZone>( scaffold_xlink, terminii );
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

MutableTreeZone::MutableTreeZone( Mutator &&base_, 
                                  vector<Mutator> &&terminii_ ) :
    base(move(base_))
{
	ASSERT( base.GetChildTreePtr() );
    for( Mutator &terminus : terminii_ )
    {
		ASSERT( terminus.GetChildTreePtr() );
		terminii.push_back(move(terminus));
	}
}


bool MutableTreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && SoloElementOf(terminii) == base;
}


size_t MutableTreeZone::GetNumTerminii() const
{
    return terminii.size();
}


TreePtr<Node> MutableTreeZone::GetBaseNode() const
{
    return base.GetChildTreePtr();
}


XLink MutableTreeZone::GetBaseXLink() const
{
	ASSERT( base.GetChildTreePtr() )(base);
    return base.GetXLink();
}


vector<XLink> MutableTreeZone::GetTerminusXLinks() const
{
	vector<XLink> xlinks;
    for( const Mutator &terminus : terminii )
		xlinks.push_back( terminus.GetXLink() );
	return xlinks;
}


XLink MutableTreeZone::GetTerminusXLink(size_t index) const
{
	return terminii[index].GetXLink();
}


const Mutator &MutableTreeZone::GetBaseMutator() const
{
    return base;
}


void MutableTreeZone::SetBaseMutator( const Mutator &new_base )
{
	ASSERT( new_base.GetChildTreePtr() );
	base = new_base;
}


const Mutator &MutableTreeZone::GetTerminusMutator(size_t index) const
{
	return terminii[index];
}


void MutableTreeZone::Exchange( FreeZone *free_zone, vector<MutableTreeZone *> fixups )
{    		
	// Should be true regardless of empty zones
	ASSERT( GetNumTerminii() == free_zone->GetNumTerminii() );

	ASSERT( !free_zone->IsEmpty() ); // Could add support but apparently don't need it rn	       	        
     
    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    vector<MutableTreeZone *>::iterator fixups_it = fixups.begin();
    FreeZone::TerminusIterator free_terminus_it = free_zone->GetTerminiiBegin();    
    for( Mutator &tree_terminus : terminii )
    {
        ASSERT( free_terminus_it != free_zone->GetTerminiiEnd() ); // length mismatch    
                         
		if( !fixups.empty() && *fixups_it )
			ASSERT( (*fixups_it)->GetBaseMutator() == tree_terminus );
                         
	    if( IsEmpty() )
	    {
			ASSERT( tree_terminus==base );
			
			// Avoid side-effects on base, at the cost of not getting updated free terminus
			tree_terminus.SetParent(*free_terminus_it);
		}	
		else
		{
			tree_terminus.ExchangeParent(*free_terminus_it); // deep
		}
                
        ASSERT( tree_terminus.GetChildTreePtr() );
        
   		if( !fixups.empty() && *fixups_it )
			(*fixups_it)->SetBaseMutator(tree_terminus);
        
        free_terminus_it++;
        if( !fixups.empty() )
			fixups_it++;
    } 
    ASSERT( free_terminus_it == free_zone->GetTerminiiEnd() ); // length mismatch  

    // Exchange the base. We want to modify the child side of the base
    // in-place, leaving valid mutators behind. 
	TreePtr<Node> original_tree_zone_base = base.GetChildTreePtr();

    TreePtr<Node> free_base = free_zone->GetBaseNode();
    (void)base.ExchangeChild( free_base );	// deep 
    
    if( original_tree_zone_base )
		free_zone->SetBase( original_tree_zone_base );	
	else
		*free_zone = *FreeZone::CreateEmpty();		
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
