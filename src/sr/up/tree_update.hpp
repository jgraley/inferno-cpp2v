#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

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

// ------------------------- Runners --------------------------

class TreeUpdater
{
public:
	TreeUpdater(XTreeDatabase *x_tree_db = nullptr); // db optional
	
	FreeZone Evaluate( const ZoneExpression *expr );
	void TransformToIncrementalAndExecute( shared_ptr<Command> cmd );
	
private:
	XTreeDatabase * const db;
};	

}

#endif
