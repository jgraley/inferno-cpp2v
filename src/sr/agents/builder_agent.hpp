#ifndef BUILDER_AGENT_HPP
#define BUILDER_AGENT_HPP

#include "../search_replace.hpp"
#include "agent_intermediates.hpp"

namespace SR
{
    
/**
 * Agent matches basically anything and builds new subtree during replace
 */ 
struct BuilderAgent : public virtual SearchLeafAgent    
{
public:
	// TODO do this via a transformation as with TransformOf/TransformOf
    BuilderAgent() {}

    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const final;                                       

    bool ReplaceKeyerQuery( PatternLink me_plink, 
                            set<PatternLink> keyer_plinks ) final;                                  
	TreePtr<Node> BuildReplaceImpl( PatternLink me_plink, 
                                    TreePtr<Node> key_node ) final;  
                                            
    virtual TreePtr<Node> BuildNewSubtree() = 0;
};

};
#endif
