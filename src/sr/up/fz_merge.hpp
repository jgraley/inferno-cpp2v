#ifndef FZ_MERGE_HPP
#define FZ_MERGE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- FreeZoneMerger --------------------------

class FreeZoneMerger
{
public:	
	FreeZoneMerger();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<Layout> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<Layout> &root_expr );
};

}

#endif
