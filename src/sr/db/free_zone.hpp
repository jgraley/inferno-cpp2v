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
    typedef list<Mutator>::iterator TerminusIterator;
    typedef list<Mutator>::const_iterator TerminusConstIterator;

    static FreeZone CreateSubtree( TreePtr<Node> base );
    static FreeZone CreateEmpty();

    FreeZone();
    explicit FreeZone( TreePtr<Node> base, list<Mutator> &&terminii );

    FreeZone &operator=( const FreeZone &other );

    bool IsEmpty() const override;
    ContainerInterface *TryGetContainerBase() const;
    size_t GetNumTerminii() const override;
    const Mutator &GetTerminusMutator(size_t index) const;	
    TreePtr<Node> GetBaseNode() const override;
    void SetBase( TreePtr<Node> base );
    
    void MergeAll( list<unique_ptr<FreeZone>> &&child_zones );     
    TerminusIterator MergeTerminus( TerminusIterator it_t, 
                                    unique_ptr<FreeZone> &&child_zone,
                                    XLink *resulting_xlink = nullptr );
    void AddTerminus(Mutator &&terminus);      
    TerminusIterator GetTerminiiBegin();
    TerminusIterator GetTerminiiEnd();
    TerminusConstIterator GetTerminiiBegin() const;
    TerminusConstIterator GetTerminiiEnd() const;

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    list<Mutator> terminii;
};
 
}

#endif
