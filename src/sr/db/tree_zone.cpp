#include "tree_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "free_zone.hpp"

using namespace SR;


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


void TreeZone::SetTreeOrdinal(DBCommon::TreeOrdinal ordinal_)
{
	ordinal = ordinal_;
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



