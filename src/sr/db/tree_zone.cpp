#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;

// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateSubtree( XLink base, 
                                    DBCommon::TreeOrdinal ordinal_ )
{
    return TreeZone( base, vector<XLink>(), ordinal_ );
}


TreeZone TreeZone::CreateEmpty( XLink base )
{
    ASSERTS( base );
    // One element, same as base. Ordinal shall be invalid.
    return TreeZone( base, vector<XLink>{ base }, DBCommon::TreeOrdinal(-1) ); 
}


TreeZone::TreeZone( XLink base_, 
					  vector<XLink> terminii_,
					  DBCommon::TreeOrdinal ordinal_ ) :
    base( base_ ),
    terminii( terminii_ ),
	ordinal( ordinal_ )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildTreePtr() ); // Cannot be empty
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && SoloElementOf(terminii)==base;    
}


size_t TreeZone::GetNumTerminii() const
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


vector<XLink> TreeZone::GetTerminusXLinks() const
{
    return terminii;
}


XLink TreeZone::GetTerminusXLink(size_t index) const
{
	return terminii[index];
}



DBCommon::TreeOrdinal TreeZone::GetTreeOrdinal() const
{
	return ordinal;
}


void TreeZone::SetBaseXLink(XLink new_base)
{
	base = new_base;
}


unique_ptr<FreeZone> TreeZone::Duplicate() const
{
    if( IsEmpty() )
        return make_unique<FreeZone>( FreeZone::CreateEmpty() );
        
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


string TreeZone::GetTrace() const
{
    string s;
    if( IsEmpty() )
    {
		s += Trace(base);
        s += " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {        
		s += SSPrintf("T%d ", GetTreeOrdinal());
		s += Trace(base);
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "TreeZone(" + s +")";
}

// ------------------------- MutableZone --------------------------

MutableZone::MutableZone( Mutator &&base_, 
                                  vector<Mutator> &&terminii_,
                                  DBCommon::TreeOrdinal ordinal_ ) :
    base(move(base_)),
    terminii(move(terminii_)),
    ordinal( ordinal_ )
{
	ASSERT( base.GetChildTreePtr() );
    for( Mutator &terminus : terminii )
		ASSERT( terminus.GetChildTreePtr() );
}


bool MutableZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && SoloElementOf(terminii) == base;
}


TreeZone MutableZone::GetXTreeZone() const
{
    vector<XLink> v;
    for( const Mutator &m : terminii )
		v.push_back( m.GetXLink() );
    return TreeZone( base.GetXLink(), v, ordinal );    
}


void MutableZone::Swap( MutableZone &tree_zone_l, vector<TreeZone *> fixups_l, 
                        MutableZone &tree_zone_r, vector<TreeZone *> fixups_r )
{
	// Should be true regardless of empty zones
	ASSERTS( tree_zone_l.terminii.size() == tree_zone_r.terminii.size() );
	ASSERTS( !tree_zone_l.IsEmpty() ); 
	ASSERTS( !tree_zone_r.IsEmpty() );       

    // Do a co-walk and exchange one at a time. We want to modify the parent
    // sides of the terminii in-place, leaving valid mutators behind. 
    vector<TreeZone *>::iterator fixups_l_it = fixups_l.begin();
    vector<TreeZone *>::iterator fixups_r_it = fixups_r.begin();
    vector<Mutator>::iterator terminus_it_r = tree_zone_r.terminii.begin();    
    for( Mutator &terminus_l : tree_zone_l.terminii )
    {
        ASSERTS( terminus_it_r != tree_zone_r.terminii.end() ); // length mismatch    
        Mutator &terminus_r = *terminus_it_r;
         
		if( !fixups_l.empty() && *fixups_l_it )
			ASSERTS( (*fixups_l_it)->GetBaseXLink() == terminus_l.GetXLink() );
		if( !fixups_r.empty() && *fixups_r_it )
			ASSERTS( (*fixups_r_it)->GetBaseXLink() == terminus_r.GetXLink() );
                         
	    terminus_l.ExchangeParent(terminus_r); // deep
		               
        ASSERTS( terminus_l.GetChildTreePtr() );
        ASSERTS( terminus_r.GetChildTreePtr() );
        
   		if( !fixups_l.empty() && *fixups_l_it )
			(*fixups_l_it)->SetBaseXLink(terminus_l.GetXLink());
   		if( !fixups_r.empty() && *fixups_r_it )
			(*fixups_r_it)->SetBaseXLink(terminus_r.GetXLink());
        
        terminus_it_r++;
        if( !fixups_l.empty() )
			fixups_l_it++;
        if( !fixups_r.empty() )
			fixups_r_it++;
    } 
    ASSERTS( terminus_it_r == tree_zone_r.terminii.end() ); // length mismatch    
	
    TreePtr<Node> base_node_r = tree_zone_r.base.GetChildTreePtr();
    TreePtr<Node> original_base_node_l = tree_zone_l.base.ExchangeChild( base_node_r );	// deep 
	(void)tree_zone_r.base.ExchangeChild( original_base_node_l );		
}


string MutableZone::GetTrace() const
{
    string s;	
    if( IsEmpty() )
    {
		s += Trace(base);
        s += " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {       
		s += SSPrintf("T%d ", ordinal);
		s += Trace(base);
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
    
    return "MutableZone(" + s +")";
}




RAIISuspendForSwapBase::RAIISuspendForSwapBase( TreeZone &zone1_, TreeZone &zone2_ ) :
	zone1(zone1_),
	zone2(zone2_)
{
}

				                
RAIISuspendForSwapBase::~RAIISuspendForSwapBase()
{
}
