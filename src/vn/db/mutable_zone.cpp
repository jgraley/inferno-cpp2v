#include "mutable_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace VN;

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
                         
	    Mutator::SwapParents( terminus_l, terminus_r ); // deep
		               
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

