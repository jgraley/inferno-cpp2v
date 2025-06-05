#ifndef COMPLEMENT_PASS_HPP
#define COMPLEMENT_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/**
 * Find Tree Zones that make up the set-complement of the Tree Zones in 
 * the layout. We wont' necessarily get this right by taking cues from the
 * layout as with gap-finding and inversion. For example, an apparent gap 
 * could fully and strictly contain a TZ that's moved somewhere else in the
 * layout. Here, we analyse the tree zones without reference to the layout.
 * 
 * We will go right ahead and remove all intrinsic DB content on the 
 * complement zones. Because we only check tree zones, this can include 
 * nodes that are present in free zones in the layout. So we need to do this
 * before adding the free zones to the intrinsic DB otherwise they could be
 * added and then deleted again.
 * 
 * OBSOLETE - we now delete intrisic DB info as a post-pass after inversion
 */
class ComplementPass
{
public:
    typedef map<XLink, TreeZone *, DepthFirstRelation> DepthFirstOrderingZones;

    ComplementPass( XTreeDatabase *db );
    
    void Run(const Mutator &origin_mutator, shared_ptr<Patch> source_layout);
        
private:
    void WalkTreeZones(XLink target_base);
    void CreateComplementTZ(XLink target_base);

    XTreeDatabase * const db;
    const DepthFirstRelation df_rel;
    DepthFirstOrderingZones source_tzs_df_by_base;
};

}

#endif
