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
    
private:
    void DetermineDomain( PatternLink root_plink, XLink root_xlink );
    void ExtendDomain( PatternLink plink );

public:
    // Global domain of possible xlink values
    unordered_set<XLink> domain;            
    
    // SimpleCompare equivalence classes over the domain.
    shared_ptr<QuotientSet> domain_extension_classes;
    
    // Child-to-parent map
    unordered_map<XLink, XLink> parents;
};    
    
};

#endif
