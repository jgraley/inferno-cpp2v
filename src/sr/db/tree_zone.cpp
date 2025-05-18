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


FreeZone MutableTreeZone::Exchange( const FreeZone &new_free_zone, vector<MutableTreeZone *> fixups )
{	
	// Should be true regardless of empty zones
	ASSERT( GetNumTerminii() == new_free_zone.GetNumTerminii() );

	ASSERT( !new_free_zone.IsEmpty() ); // Could add support but apparently don't need it rn	       	        
     
	// Get a non-const version so we can start changing the terminii etc
	FreeZone free_zone = new_free_zone;
	
    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    vector<MutableTreeZone *>::iterator fixups_it = fixups.begin();
    FreeZone::TerminusIterator free_terminus_it = free_zone.GetTerminiiBegin();    
    for( Mutator &tree_terminus : terminii )
    {
        ASSERT( free_terminus_it != free_zone.GetTerminiiEnd() ); // length mismatch    
                         
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
    ASSERT( free_terminus_it == free_zone.GetTerminiiEnd() ); // length mismatch  

    // Exchange the base. We want to modify the child side of the base
    // in-place, leaving valid mutators behind. 
	TreePtr<Node> original_tree_zone_base = base.GetChildTreePtr();

    TreePtr<Node> free_base = free_zone.GetBaseNode();
    (void)base.ExchangeChild( free_base );	// deep 
    
    if( original_tree_zone_base )
		free_zone.SetBase( original_tree_zone_base );	
	else
		free_zone = *FreeZone::CreateEmpty();		
		
	return free_zone;
}


void MutableTreeZone::Swap( TreeZone &tree_zone_r, vector<MutableTreeZone *> fixups_l, vector<MutableTreeZone *> fixups_r )
{
	// Should be true regardless of empty zones
	ASSERT( GetNumTerminii() == tree_zone_r.GetNumTerminii() );
	ASSERT( !IsEmpty() ); // TODO overcautious?      
	ASSERT( !tree_zone_r.IsEmpty() ); // TODO overcautious?      

	auto mutable_tree_zone_r = dynamic_cast<MutableTreeZone &>(tree_zone_r);

    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    vector<MutableTreeZone *>::iterator fixups_l_it = fixups_l.begin();
    vector<MutableTreeZone *>::iterator fixups_r_it = fixups_r.begin();
    vector<Mutator>::iterator terminus_it_r = mutable_tree_zone_r.terminii.begin();    
    for( Mutator &terminus_l : terminii )
    {
        ASSERT( terminus_it_r != mutable_tree_zone_r.terminii.end() ); // length mismatch    
        Mutator &terminus_r = *terminus_it_r;
         
		if( !fixups_l.empty() && *fixups_l_it )
			ASSERT( (*fixups_l_it)->GetBaseMutator() == terminus_l );
		if( !fixups_r.empty() && *fixups_r_it )
			ASSERT( (*fixups_r_it)->GetBaseMutator() == terminus_r );
                         
	    terminus_l.ExchangeParent(terminus_r); // deep
		               
        ASSERT( terminus_l.GetChildTreePtr() );
        ASSERT( terminus_r.GetChildTreePtr() );
        
   		if( !fixups_l.empty() && *fixups_l_it )
			(*fixups_l_it)->SetBaseMutator(terminus_l);
   		if( !fixups_r.empty() && *fixups_r_it )
			(*fixups_r_it)->SetBaseMutator(terminus_r);
        
        terminus_it_r++;
        if( !fixups_l.empty() )
			fixups_l_it++;
        if( !fixups_r.empty() )
			fixups_r_it++;
    } 
    ASSERT( terminus_it_r == mutable_tree_zone_r.terminii.end() ); // length mismatch    
	
    TreePtr<Node> base_node_r = mutable_tree_zone_r.GetBaseNode();
    TreePtr<Node> original_base_node_l = base.ExchangeChild( base_node_r );	// deep 
	(void)mutable_tree_zone_r.base.ExchangeChild( original_base_node_l );		
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
