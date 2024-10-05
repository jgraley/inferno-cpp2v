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

    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const final;                                       

    bool ReplaceKeyerQuery( PatternLink me_plink, 
                            set<PatternLink> keyer_plinks ) final;                                  

    ReplaceExprPtr GenReplaceExprImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink ) final;
                                            
    virtual TreePtr<Node> BuildNewSubtree() = 0;
};

};
#endif
