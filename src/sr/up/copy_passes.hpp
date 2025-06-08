#ifndef COPY_PASSES_HPP
#define COPY_PASSES_HPP

#include "patches.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/** 
 * Determine by examining tree patches with intent DEFAULT and MOVABLE
 * how to copy N-1 of any set of N duplicates such that as much as possible
 * the ones we choose were already MOVABLE. Set their intent to COPYABLE.
 */
class ChooseCopiesPass
{
public:        
    // Can change the supplied shared ptr
    void Run(shared_ptr<Patch> &layout);
};


/**
 * Duplicate the zones in the COPYABLE tree zone patches and thereby turn 
 * them into free zone patches.
 */
class CopyingPass
{
public:        
    // Can change the supplied shared ptr
    void Run(shared_ptr<Patch> &layout);
};


}

#endif
