#ifndef DELTA_AGENT_HPP
#define DELTA_AGENT_HPP

#include "common/common.hpp"
#include "autolocating_agent.hpp"
#include "../scr_engine.hpp" 

namespace VN
{ 

/// Agent that is used in a combined search/replace path to seperate search 
/// and replace sub-patterns. During search, the `through` pointer will be 
/// followed and the sub-pattern found there must match. During replace, the 
/// sub-pattern under `overlay` is used to reconstruct the output tree.
/// If the replace pattern under `overlay` contains wildcards, these will 
/// be overlayed over the pattern under `through`, to the extent that this
/// is unambiguous and the structures of the two subtrees match.
class DeltaAgent : public virtual AutolocatingAgent,
                   public StartsOverlay
{  
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                
    virtual list<PatternLink> GetVisibleChildren( Path v ) const;

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink,
                                                  const SCREngine *acting_engine ) final;

    virtual void StartPlanOverlay(SCREngine *acting_engine);
    
private:
	Syntax::Production GetAgentProduction() const final;
	string GetRender( const RenderKit &kit, string prefix, Syntax::Production surround_prod ) const final;
    virtual const TreePtrInterface *GetThrough() const = 0;
    virtual const TreePtrInterface *GetOverlay() const = 0;    
    virtual NodeBlock GetGraphBlockInfo() const;
};


template<class PRE_RESTRICTION>
class Delta : public DeltaAgent, 
              public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<PRE_RESTRICTION> through;
    TreePtr<PRE_RESTRICTION> overlay;
    virtual const TreePtrInterface *GetThrough() const override
    {
        return &through;
    }
    virtual const TreePtrInterface *GetOverlay() const override
    {
        return &overlay;
    }
};

};

#endif
