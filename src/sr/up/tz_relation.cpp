#include "tz_relation.hpp"

#include "helpers/flatten.hpp"
#include "db/df_relation.hpp"
#include "db/x_tree_database.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"

using namespace SR;

TreeZoneRelation::TreeZoneRelation(const XTreeDatabase *db_) :
    db( db_ )
{
}


bool TreeZoneRelation::operator()( const TreeZone &l, const TreeZone &r ) const
{
    return Compare3Way(l, r) < 0;
}


bool TreeZoneRelation::CompareEqual( const TreeZone &l, const TreeZone &r ) const
{
	if( l.GetBaseXLink() != r.GetBaseXLink() )
		return false;
	
	if( l.GetTerminusXLinks() != r.GetTerminusXLinks() )
		return false;
	
	return true;
}


Orderable::Diff TreeZoneRelation::Compare3Way( const TreeZone &l, const TreeZone &r ) const
{
	return CompareHierarchical( l, r ).first;
}



pair<Orderable::Diff, ZoneRelation::RelType> TreeZoneRelation::CompareHierarchical( const TreeZone &l, const TreeZone &r ) const 
{
    TRACES("l=")(l)(" r=")(r);	
	DepthFirstRelation df_rel(db);
	
    const TreeZone *d;
    const TreeZone *a;        
    auto p_base = df_rel.CompareHierarchical(l.GetBaseXLink(), r.GetBaseXLink());
    switch( p_base.second )
    {
        case DepthFirstRelation::EQUAL:
        {
			// Bases are the same so what about the terminii?
			Orderable::Diff term_diff = STLCompare3Way(l.GetTerminusXLinks(), r.GetTerminusXLinks());
			if( term_diff==0 )
				return make_pair(term_diff, EQUAL);
			else
				return make_pair(term_diff, OVERLAP_TERMINII);
		}
			
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
            return make_pair(p_base.first, DISTINCT_SIBLINGS); // weakly removed sibling bases cannot overlap
            
        default:
            ASSERTFAILS();
    }
    
	TRACES(" same branch a=")(a)(" d=")(d);

    // If a has a terminus that is an ancestor (weakly)
    // to d's base, then they do not overlap. Otherwise, they do.
    for( XLink terminus : a->GetTerminusXLinks() )
    {
		TRACES(" terminus: ")(terminus);
        auto p_term = df_rel.CompareHierarchical(terminus, d->GetBaseXLink());
        TRACES(" ")(p_term);
        switch( p_term.second )
        {
            case DepthFirstRelation::EQUAL:
				TRACES(" touching: no overlap\n");
                return make_pair(p_base.first, DISTINCT_SUBTREE); // close but no overlap, zone d begins at from a's terminus
                
            case DepthFirstRelation::LEFT_IS_ANCESTOR:
				TRACES(" aa dd: no overlap\n");
                return make_pair(p_base.first, DISTINCT_SUBTREE); // no overlap, zone d descends from a's terminus
                
            case DepthFirstRelation::RIGHT_IS_ANCESTOR:
				TRACES(" adad: overlap\n");
                return make_pair(p_base.first, OVERLAP_GENERAL); // a's terminus descends from base of d, making the zones overlap
            
            case DepthFirstRelation::CONTAINER_SIBLINGS:
            case DepthFirstRelation::ITEM_SIBLINGS:
            case DepthFirstRelation::ROOT_SIBLINGS:
                break; // a's terminus is on a different branch than d
                
            default:
                ASSERTFAILS();
        }                
    }
    
	TRACES(" add: overlap\n");
    return make_pair(p_base.first, OVERLAP_GENERAL); // None of a's terminii in path from a's base to zone d, so a goes to leaf through d
}
