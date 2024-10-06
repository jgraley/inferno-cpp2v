#ifndef TZ_OVERLAP_HPP
#define TZ_OVERLAP_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "commands.hpp"

namespace SR 
{

// ------------------------- TreeZoneOverlapHandler --------------------------

// To optimise:
// - Make a std::set/multiset of TZs ordered using TreeZoneRelation (idea 
//   is that the order in the replace expression is irrelevent to this algo)
// - Add a GetLastDescendant() so can form ranges for descendants which
//   are the overlap candidates.
// - TreeZoneRelation is secondary on terminii. Get matching base ranges 
//   by just depth-first-incrementing the base for an exclusive end.
// - Identical TZs are equal in TreeZoneRelation so if regular std::set 
//   is used, they will be dropped and if std::multiset they will be kept.

class TreeZoneOverlapHandler 
{
public:
	TreeZoneOverlapHandler( const XTreeDatabase *db );
	void Run( shared_ptr<ZoneExpression> &root_expr );
	void Check( shared_ptr<ZoneExpression> &root_expr );

private:
	const XTreeDatabase * const db;
};

}

#endif
