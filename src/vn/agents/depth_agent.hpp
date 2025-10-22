#ifndef DEPTH_AGENT_HPP
#define DEPTH_AGENT_HPP

#include "agent_intermediates.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "special_agent.hpp"

namespace VN
{ 

//---------------------------------- DepthAgent ------------------------------------    

/// Agent that tries to match the sub-pattern under `terminus` to any element
/// within some container of nodes. 
class DepthAgent : public virtual ColocatingAgent
{
public:
    class TerminusMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink,
                                                  const SCREngine *acting_engine ) final;

    virtual void PatternQueryRestrictions( shared_ptr<PatternQuery> ) const {};
    NodeBlock GetGraphBlockInfo() const override;

    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
    virtual const TreePtrInterface *GetTerminus() const
    {
        return &terminus;
    }
};

//---------------------------------- ChildAgent ------------------------------------    

/// Agent that matches the sub-pattern under `terminus` to any child of the
/// current node. This includes children in `Conbtainer`s. It can be used to 
/// move "one level" down a tree with no assumption as to the nature of the node
/// being bypassed.
class ChildAgent : public DepthAgent
{
    class NoParentMismatch : public Mismatch {};
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;
};


/// Agent that matches any single node, with terminus support
template<class PRE_RESTRICTION>
class Child : public ChildAgent, 
                public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }    
    string GetCouplingNameHint() const final
    {
		return "child"; 
	}  
};

//---------------------------------- StuffAgent ------------------------------------    

/// Agent that matches the sub-pattern under `terminus` to any node in 
/// the subtree at current node. The current node itself may be matched. It 
/// can be used to move zero or mode "levels" down a tree with no assumption 
/// as to the nature of the node being bypassed.
/// The `recurse_restriction` may be set to a sub-pattern that must match each
/// intermediate node traversed, excluding the current node and the terminus
/// (so the `recurse_restriction` only restricts when 3 or more levels are 
/// traversed).
/// The recurse restriction is an abnormal context because it can match zero or more 
/// different subtrees.
class StuffAgent : public DepthAgent
{
public:
    virtual void PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           PatternLink keyer_plink,
                                           const XTreeDatabase *x_tree_db ) const;                                              
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;

    TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
};


template<class PRE_RESTRICTION>
class Stuff : public StuffAgent, 
              public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    string GetCouplingNameHint() const final
    {
		return "stuff"; 
	}  
};

};

#endif
