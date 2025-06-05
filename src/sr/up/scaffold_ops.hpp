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

namespace SR 
{


class ScaffoldOps
{
public:	
	ScaffoldOps( XTreeDatabase *db );

	// Take a free zone and place it inside a new extra tree, plugging its terminii so the tree is legal.
	// Generate and return a tree zone in that new extra tree, with therminii present (not plugged).
	XTreeZone FreeZoneIntoExtraTree( FreeZone free_zone, const XTreeZone &reference_tree_zone );
	
	// Fit a tree zone around a scaffold node, matching base and terminii
	XTreeZone TreeZoneAroundScaffoldNode( TreePtr<Node> scaffold_node, DBCommon::TreeOrdinal tree_ordinal );

	// Create a scaffold node siliar to supplied zone, matching base and terminii
	FreeZone CreateSimilarScaffoldZone( const Zone &zone ) const;

    FreeZone CreateScaffoldToSpec( TreePtr<Node> base, int num_terminii ) const;
    
private:
	XTreeDatabase * const db;
};

}

#endif
