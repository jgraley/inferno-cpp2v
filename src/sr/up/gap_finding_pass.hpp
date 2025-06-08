#ifndef GAP_FINDING_PASS_HPP
#define GAP_FINDING_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "patches.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/**
 * Find non-contiguous gaps between tree zones. Insert empty free zones into 
 * these gaps. This permits Inversion to clear away unused parts of the tree.
 * See #754 
 **/ 
class GapFindingPass
{
public:
	GapFindingPass();
	
	void Run(shared_ptr<Patch> source_layout);
		
private:
	void CheckPatch(shared_ptr<TreePatch> patch);
};

}

#endif
