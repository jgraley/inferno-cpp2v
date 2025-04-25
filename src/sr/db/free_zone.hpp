#ifndef FREE_ZONE_HPP
#define FREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "mutator.hpp"

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
    typedef list<shared_ptr<Mutator>>::iterator TerminusIterator;

    static unique_ptr<FreeZone> CreateSubtree( TreePtr<Node> base );
    static unique_ptr<FreeZone> CreateEmpty();
    static unique_ptr<FreeZone> CreateScaffold(const TreePtrInterface *tpi_base, int num_terminii);

    FreeZone();
    explicit FreeZone( TreePtr<Node> base, list<Mutator> &&terminii );

    FreeZone &operator=( const FreeZone &other );

    bool IsEmpty() const override;
    ContainerInterface *TryGetContainerBase() const;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;
    void SetBase( TreePtr<Node> base );
    
    void MergeAll( list<unique_ptr<FreeZone>> &&child_zones );     
    TerminusIterator MergeTerminus( TerminusIterator it_t, 
                                    unique_ptr<FreeZone> &&child_zone );
    void AddTerminus(Mutator &&terminus);      
    TerminusIterator GetTerminiiBegin();
    TerminusIterator GetTerminiiEnd();

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    list<shared_ptr<Mutator>> terminii;
};
 
}

#endif
