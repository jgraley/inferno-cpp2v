#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- Runners --------------------------

class TreeUpdater
{
public:
	TreeUpdater(XTreeDatabase *x_tree_db = nullptr); // db optional
	
	// Evaluate will always duplicate tree zones, but does not require db.
	// Returns a free zone.
	unique_ptr<FreeZone> Evaluate( shared_ptr<const ZoneExpression> expr );
	
	// Much as the name suggests. Db required and acts directly on it.
	void TransformToIncrementalAndExecute( TreeZone target_tree_zone, shared_ptr<ZoneExpression> source_layout );
	
private:
	XTreeDatabase * const db;
};	

}

#endif
