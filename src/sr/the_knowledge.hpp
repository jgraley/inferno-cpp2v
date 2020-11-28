#ifndef THE_KNOWLEDGE_HPP
#define THE_KNOWLEDGE_HPP

#include "link.hpp"
#include "common/standard.hpp"

#include <unordered_set>


/// SR namespace contains the search and replace implementation
namespace SR 
{
class QuotientSet;
    
class TheKnowledge : public Traceable
{
public:
    void Build( PatternLink root_plink, XLink root_xlink );
    enum SubtreeMode
    {
        // Behaviour for main domain population: we will check uniqueness
        // of the XLinks we meed during our recursive walk.
        REQUIRE_SOLO,
        
        // Behaviour for domain extensions. We will continue as long as 
        // nodes are not already in the domain. If a node is in the 
        // domain, we don't recurse into it since everything under it
        // will also be in the domain.
        // https://github.com/jgraley/inferno-cpp2v/issues/213#issuecomment-728266001
        STOP_IF_ALREADY_IN
    };
        
    class Nugget : public Traceable
    {
    public:
        enum Cadence
        {
            ROOT,
            SINGULAR,
            IN_SEQUENCE,
            IN_COLLECTION
        };
        typedef int IndexType;
        Cadence cadence;
        XLink parent_xlink = XLink();
        ContainerInterface *container = nullptr;
        ContainerInterface::iterator iterator;
        IndexType index = -1;
        
        string GetTrace() const;
    };
    
    const Nugget &GetNugget(XLink xlink) const;

private:
    void DetermineDomain( PatternLink root_plink, XLink root_xlink );
    void ExtendDomain( PatternLink plink );
    void AddSubtree( SubtreeMode mode, XLink root_xlink );
    void AddLink( SubtreeMode mode, 
                  XLink xlink, 
                  Nugget nugget );
    void AddChildren( SubtreeMode mode, XLink xlink );
    void AddSingularNode( SubtreeMode mode, const TreePtrInterface *x_sing, XLink xlink );
    void AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink );
    void AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink );

public:
    // Global domain of possible xlink values
    unordered_set<XLink> domain;            
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<QuotientSet> domain_extension_classes;
    
    // Child-to-parent map
    unordered_map<XLink, Nugget> nuggets;
};    
    
};

#endif
