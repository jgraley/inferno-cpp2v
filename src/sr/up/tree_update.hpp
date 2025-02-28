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
	
	// Duplicates all the tree zones and merges everything into one
	unique_ptr<FreeZone> TransformToSingleFreeZone( shared_ptr<Layout> expr );
	
	// Much as the name suggests. Db required and acts directly on it.
	void TransformToIncrementalAndExecute( TreeZone target_tree_zone, shared_ptr<Layout> source_layout );
	
private:
	XTreeDatabase * const db;
};	

}

#endif
