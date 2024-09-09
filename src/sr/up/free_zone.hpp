#ifndef FREE_ZONE_HPP
#define FREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "updater.hpp"

#include <unordered_set>

namespace SR 
{

        
// ------------------------- FreeZone --------------------------

// FreeZone is for zones that are not anywhere in the current
// x tree. They are assumed to be "orphaned" and so for minimality
// we express the base as a TreePtr<Node>. See #623.
class FreeZone : public Zone
{ 
public:
    static FreeZone CreateSubtree( TreePtr<Node> base );
    static FreeZone CreateEmpty();

	FreeZone();
    explicit FreeZone( TreePtr<Node> base, vector<shared_ptr<Updater>> terminii );

    bool IsEmpty() const override;
	int GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    void AddTerminus(int ti, shared_ptr<Updater> terminus);      
    vector<shared_ptr<Updater>> GetTerminusUpdaters() const;
    shared_ptr<Updater> GetTerminusUpdater(int ti) const;
    void DropTerminus(int ti);
    
    void Join( FreeZone &child_zone, int terminus_index );

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    map<int, shared_ptr<Updater>> terminii;
};
 
}

#endif
