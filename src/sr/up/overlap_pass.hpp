#ifndef OVERLAP_PASS_HPP
#define OVERLAP_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{


class OverlapPass 
{
public:
    OverlapPass( const XTreeDatabase *db );
    void Run( shared_ptr<Patch> &layout );
    void Check( shared_ptr<Patch> &layout );

private:
    const XTreeDatabase * const db;
};

// To optimise:
// - Make a std::set/multiset of TZs ordered using TreeZoneRelation (idea 
//   is that the order in the replace layout is irrelevent to this algo)
// - Add a GetLastDescendantXLink() so can form ranges for descendants which
//   are the overlap candidates.
// - TreeZoneRelation is secondary on terminii. Get matching base ranges 
//   by just depth-first-incrementing the base for an exclusive end.
// - Identical TZs are equal in TreeZoneRelation so if regular std::set 
//   is used, they will be dropped and if std::multiset they will be kept.



}

#endif
