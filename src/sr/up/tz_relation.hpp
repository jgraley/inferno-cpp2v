#ifndef TZ_RELATION_HPP
#define TZ_RELATION_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "terminus.hpp"

namespace SR 
{
class XTreeDatabase;
class TreeZone;

// ------------------------- PointeeRelation --------------------------

class ZoneRelation
{
public:
	enum RelType
	{
		EQUAL,              // Base and terminii are the same 
		OVERLAP_GENERAL,   // overlapping, base differs and terminii may differ
		OVERLAP_TERMINII,  // overlapping, terminii differ 
		DISTINCT_SUBTREE,  // no overlap due to terminus  (i.e. "Ab->At->Db->Dt?")
		DISTINCT_SIBLINGS  // no overlap, on different branches 
	};
};

// ------------------------- TreeZoneRelation --------------------------

class TreeZoneRelation : public ZoneRelation
{
public:
	typedef TreeZone Domain;
	
    TreeZoneRelation(const XTreeDatabase *db);

    /// Less operator: for use with set, map etc
    bool operator()( const TreeZone &l, const TreeZone &r ) const;
    Orderable::Diff Compare3Way( const TreeZone &l, const TreeZone &r ) const;
	bool CompareEqual( const TreeZone &l, const TreeZone &r ) const;
    pair<Orderable::Diff, RelType> CompareHierarchical( const TreeZone &l, const TreeZone &r ) const;
    
private:
	const XTreeDatabase * const db;
}; 
	
// ------------------------- PointeeRelation template --------------------------

template<typename POINTEE_REL>
class PointeeRelation
{
public:	
	typedef POINTEE_REL PointeeRel;
	PointeeRelation( const POINTEE_REL &pointee_rel_ ) :
		pointee_rel( pointee_rel_ )
	{
	}
	
	bool operator()( const typename PointeeRel::Domain *l, 
	                 const typename PointeeRel::Domain *r )
	{
		return pointee_rel( *l, *r );
	}
	
    Orderable::Diff Compare3Way( const typename PointeeRel::Domain *l, 
	                             const typename PointeeRel::Domain *r ) const
	{
		ASSERT(l);
		ASSERT(r);
		
		return pointee_rel.Compare3Way( *l, *r );				
	}
	
private:
	const PointeeRel &pointee_rel;
};
	
}

#endif
