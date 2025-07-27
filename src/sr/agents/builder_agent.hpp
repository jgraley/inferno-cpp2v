#ifndef BUILDER_AGENT_HPP
#define BUILDER_AGENT_HPP

#include "../search_replace.hpp"
#include "agent_intermediates.hpp"

namespace SR
{
    
/**
 * Agent matches basically anything and builds new subtree during replace
 */ 
struct BuilderAgent : public virtual NonlocatingAgent    
{
public:
    // TODO do this via a transformation as with TransformOf/TransformOf
    BuilderAgent() {}

    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const final;                                                                       

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink,
                                                  const SCREngine *acting_engine ) final;
                                            
    virtual TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) = 0;
    
private:
    TreePtr<Node> built_node = nullptr;    
};

};
#endif
