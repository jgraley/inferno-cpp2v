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

FreeZone RunForBuilder( const ZoneExpression *expr );
void RunForReplace( Command *cmd, XTreeDatabase *x_tree_db );
	

}

#endif
