#ifndef MOVE_IN_PASS_HPP
#define MOVE_IN_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "patches.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/**
 * TODO
 **/ 
class MoveInPass
{
public:
	MoveInPass( XTreeDatabase *db );
	
	void Run(MovesMap &moves_map);

private:
	XTreeDatabase * const db;
};

}

#endif
