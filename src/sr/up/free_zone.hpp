#ifndef FREE_ZONE_HPP
#define FREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "terminus.hpp"

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
    explicit FreeZone( TreePtr<Node> base, list<shared_ptr<Terminus>> &&terminii );

	FreeZone &operator=( const FreeZone &other );

    bool IsEmpty() const override;
	int GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;
    void PopulateAll( list<unique_ptr<FreeZone>> &&child_zones );     
	list<shared_ptr<Terminus>>::iterator PopulateTerminus( list<shared_ptr<Terminus>>::iterator it_t, 
                                                           unique_ptr<FreeZone> &&child_zone );
    void AddTerminus(shared_ptr<Terminus> terminus);      
    list<shared_ptr<Terminus>> &GetTerminii();

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    list<shared_ptr<Terminus>> terminii;
};
 
}

#endif
