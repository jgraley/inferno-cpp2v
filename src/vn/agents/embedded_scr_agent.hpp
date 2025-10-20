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
    EmbeddedSCRAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search );
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;              
    virtual const TreePtrInterface *GetThrough() const = 0;    
    virtual void MaybeChildrenPlanOverlay( SCREngine *acting_engine,
                                           PatternLink me_plink, 
                                           PatternLink bottom_layer_plink );

    list<PatternLink> GetVisibleChildren( Path v ) const override;
    virtual bool IsSearch() const;
        
    TreePtr<Node> GetEmbeddedSearchPattern() const override;
    TreePtr<Node> GetEmbeddedReplacePattern() const override;
    
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const override;
    
    virtual TreePtr<Node> EvolveIntoEmbeddedCompareReplace() = 0;

    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;   
    
private:
    bool is_search;
};


/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<class PRE_RESTRICTION>
class EmbeddedSCR : public EmbeddedSCRAgent, 
                    public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    // EmbeddedSearchReplace must be constructed using constructor
    EmbeddedSCR( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
        EmbeddedSCRAgent( sp, rp, is_search ),
        through( t )
    {
    }

    TreePtr<PRE_RESTRICTION> through;
    virtual const TreePtrInterface *GetThrough() const
    {
        return &through;
    }
    
    TreePtr<Node> EvolveIntoEmbeddedCompareReplace() override;
};


/// EmbeddedSCR that performs a seperate compare and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class EmbeddedCompareReplace : public EmbeddedSCR<PRE_RESTRICTION>
{
public:
    EmbeddedCompareReplace() : EmbeddedSCR<PRE_RESTRICTION>( nullptr, nullptr, nullptr, false ) {}      
    EmbeddedCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCR<PRE_RESTRICTION>( t, sp, rp, false ) {}
    string GetToken() const final
    {
		return "embedded"; 
	}
};


/// EmbeddedSCR that performs a seperate search and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class EmbeddedSearchReplace : public EmbeddedSCR<PRE_RESTRICTION>
{
public:
    EmbeddedSearchReplace() : EmbeddedSCR<PRE_RESTRICTION>( nullptr, nullptr, nullptr, true ) {}      
    EmbeddedSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCR<PRE_RESTRICTION>( t, sp, rp, true ) {}
};


template<class PRE_RESTRICTION>
TreePtr<Node> EmbeddedSCR<PRE_RESTRICTION>::EvolveIntoEmbeddedCompareReplace()
{
    return MakePatternNode<EmbeddedCompareReplace<PRE_RESTRICTION>>( through, search_pattern, replace_pattern );
}

};
#endif
