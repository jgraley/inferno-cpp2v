#ifndef SCAFFOLD_OPS_HPP
#define SCAFFOLD_OPS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../db/db_common.hpp"
#include <functional>

namespace VN 
{

// ------------------------- ScaffoldOps --------------------------

class ScaffoldOps
{
public:	
	ScaffoldOps( XTreeDatabase *db );

	/** 
	 * Take a free zone and place it inside a new extra tree, plugging its terminii so the tree is legal.
	 * Generate and return a tree zone in that new extra tree, with the correct terminii (not plugged).
	 * Type correctness: we get the plug node types from tree_zone_for_plug_types.
	 */ 
	TreeZone FreeZoneIntoExtraTree( FreeZone free_zone, const TreeZone &tree_zone_for_plug_types );
	
	/** 
	 * Fit a tree zone around a scaffold node, matching base and terminii
	 */ 
	TreeZone TreeZoneAroundScaffoldNode( TreePtr<Node> scaffold_node, DBCommon::TreeOrdinal tree_ordinal );

	/** 
	 * Create a scaffold node simliar to supplied zone, matching base and terminii. Return the
	 * scaffold inside a free zone.
	 */ 
	FreeZone CreateSimilarScaffoldZone( const Zone &zone ) const;

private:
    FreeZone CreateScaffoldToSpec( TreePtr<Node> base, int num_terminii ) const;
    
	XTreeDatabase * const db;
};

}

#endif
