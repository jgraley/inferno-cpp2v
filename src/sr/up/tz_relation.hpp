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

class ZoneRelation
{
public:
	enum RelType
	{
		SAME,              // Base and terminii are the same 
		OVERLAP_GENERAL,   // overlapping, base differs and terminii may differ
		OVERLAP_TERMINII,  // overlapping, terminii differ 
		DISTINCT_SUBTREE,  // no overlap due to terminus  (i.e. "Ab->At->Db->Dt?")
		DISTINCT_SIBLINGS  // no overlap, on different branches 
	};
};


class TreeZoneRelation : public ZoneRelation
{
public:
    TreeZoneRelation(const XTreeDatabase *db);

    /// Less operator: for use with set, map etc
    bool operator()( const TreeZone &l, const TreeZone &r ) const;
    Orderable::Diff Compare3Way( const TreeZone &l, const TreeZone &r ) const;
	bool CompareEqual( const TreeZone &l, const TreeZone &r ) const;
    pair<Orderable::Diff, RelType> CompareHierarchical( const TreeZone &l, const TreeZone &r ) const;
    
private:
	const XTreeDatabase * const db;
}; 
	
	
	
	
}

#endif
