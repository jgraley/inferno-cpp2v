#ifndef COMPLEMENT_HPP
#define COMPLEMENT_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// Find Tree Zones that complement the Tree Zones in the layout.
class TreeZoneComplementer
{
public:
    typedef map<XLink, TreeZone *, DepthFirstRelation> DepthFirstOrderingZones;

    TreeZoneComplementer( XTreeDatabase *db );
    
    void Run(XLink target_origin, shared_ptr<Patch> source_layout);
    
    const list<XTreeZone> &GetComplement() const;
    
private:
    void WalkTreeZones(XLink target_base);
    void CreateComplementTZ(XLink target_base);

    XTreeDatabase * const db;
    const DepthFirstRelation df_rel;
    DepthFirstOrderingZones source_tzs_df_by_base;
};

}

#endif
