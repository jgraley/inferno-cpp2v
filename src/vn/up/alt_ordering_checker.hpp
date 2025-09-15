#ifndef ALT_ORDERING_CHECKER_HPP
#define ALT_ORDERING_CHECKER_HPP

#include "patches.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace VN 
{
	
/**
 * Alternative ordering checker using a different algorithm
 */ 
class AltOrderingChecker
{
public:
    AltOrderingChecker(const XTreeDatabase *db_);
    
    void Check( shared_ptr<Patch> layout );

private:
    void Worker( shared_ptr<Patch> patch, XLink x_sub_base, bool should_touch );
    void AppendNextDescendantTreePatches( shared_ptr<Patch> start_patch, 
                                          vector<shared_ptr<TreePatch>> *ndt_patches );
        
    const XTreeDatabase * const db;
    VN::DepthFirstRelation dfr;   
};

}

#endif
