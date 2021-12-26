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
    void Clear();
    enum SubtreeMode
    {
        // Behaviour for main domain population: we will check uniqueness
        // of the XLinks we meet during our recursive walk.
        REQUIRE_SOLO,
        
        // Behaviour for domain extensions. We will continue as long as 
        // nodes are not already in the domain. If a node is in the 
        // domain, we don't recurse into it since everything under it
        // will also be in the domain.
        // https://github.com/jgraley/inferno-cpp2v/issues/213#issuecomment-728266001
        STOP_IF_ALREADY_IN
    };
    
    typedef int IndexType;

    // Domain ordered by depth-first walk
    // Don't use a vector for this:
    // (a) you'd need the size in advance otherwise the iterators in
    // the nuggets will go bad while populating and
    // (b) incremental domain update will be hard
    typedef list<XLink> DepthFirstOrderedDomain;    
    typedef DepthFirstOrderedDomain::const_iterator DepthFirstOrderedIt;    
    
    class Nugget : public Traceable
    {
    public:
        enum ContainmentContext
        {
            ROOT,
            SINGULAR,
            IN_SEQUENCE,
            IN_COLLECTION
        };

        ContainmentContext containment_context;
        
        // Parent X link if not ROOT
        // Note that the parent is unique because:
        // - nugget is relative to a link, not a node,
        // - multiple parents only allowed at leaf, and parent is 
        //   (at least) one level back from that.
        XLink parent_xlink = XLink();
        
        // Last of the descendents in depth first order
        XLink last_descendant_xlink = XLink();
        
        // First element of container of which I'm a member. 
        // Defined for all item types.
        XLink my_container_front = XLink();
        XLink my_container_back = XLink();

        // Successor element within my sequence (sequences only)
        XLink my_sequence_successor = XLink();

        // Index in a depth-first walk
        IndexType depth_first_index = -1;
        
        // Iterator in a depth-first walk
        DepthFirstOrderedIt depth_first_ordered_it;
        
        // Iterator on my_container that dereferneces to me, if 
        // IN_SEQUENCE or IN_COLLECTION. Note: only used in regeneration
        // queries.
        ContainerInterface::iterator my_container_it;
        
        string GetTrace() const;
    };
    
    const Nugget &GetNugget(XLink xlink) const;

private:
    void DetermineDomain( PatternLink root_plink, XLink root_xlink );
    void ExtendDomain( PatternLink plink );
    void AddAtRoot( SubtreeMode mode, XLink root_xlink );
    void AddLink( SubtreeMode mode, 
                  XLink xlink, 
                  Nugget nugget );
    void AddChildren( SubtreeMode mode, XLink xlink );
    void AddSingularNode( SubtreeMode mode, const TreePtrInterface *p_x_singular, XLink xlink );
    void AddSequence( SubtreeMode mode, SequenceInterface *x_seq, XLink xlink );
    void AddCollection( SubtreeMode mode, CollectionInterface *x_col, XLink xlink );
    
public:
    // Global domain of possible xlink values
    unordered_set<XLink> unordered_domain;            
    
    // Global domain of possible xlink values - ordered
    DepthFirstOrderedDomain depth_first_ordered_domain;            
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<QuotientSet> domain_extension_classes;
    
    // Child-to-parent map
    unordered_map<XLink, Nugget> nuggets;

private:    
    // Depth-first ordering
    int current_index;
    
    // Last node to be reached and given a nugget
    XLink last_xlink;
};    
    
};

#endif
