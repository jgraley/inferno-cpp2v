#ifndef EMBEDDED_SCR_AGENT_HPP
#define EMBEDDED_SCR_AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../scr_engine.hpp" 
#include "autolocating_agent.hpp"
#include "standard_agent.hpp"
#include "special_agent.hpp"

namespace VN
{ 
class SearchReplace;

/// EmbeddedSCR agents provide an instance of a search and replace engine within 
/// an enclosing pattern as an agent in a replace context. During the enclosing
/// replace operation, the sub-pattern at `through` is used to replace at the 
/// current location. After enclosing replace has completed ("LATER" model), 
/// the embedded SCR engines operate on the tree at the position of the EmbeddedSCR
/// agent, performing search and replace operations via the `search_pattern` and 
/// `replace_pattern` pointers until no more matches are found (the usual reductive style).  
class EmbeddedSCRAgent : public virtual AutolocatingAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    EmbeddedSCRAgent();
    EmbeddedSCRAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search = false );
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;              
    virtual void MaybeChildrenPlanOverlay( SCREngine *acting_engine,
                                           PatternLink me_plink, 
                                           PatternLink bottom_layer_plink );

    list<PatternLink> GetVisibleChildren( Path v ) const override;
    virtual bool IsSearch() const;
        
    TreePtr<Node> GetEmbeddedSearchPattern() const override;
    TreePtr<Node> GetEmbeddedReplacePattern() const override;
    
    TreePtr<Node> EvolveIntoEmbeddedCompareReplace();

    
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;   
    TreePtr<Node> through;
    const TreePtrInterface *GetThrough() const
    {
        return &through;
    }
    
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const override;    
    string GetCouplingNameHint() const final
    {
		return "embedded"; 
	}    
private:
    bool is_search;
};

};
#endif
