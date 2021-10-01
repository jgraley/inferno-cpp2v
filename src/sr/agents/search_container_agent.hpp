#ifndef SEARCH_CONTAINER_AGENT_HPP
#define SEARCH_CONTAINER_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

//---------------------------------- SearchContainerAgent ------------------------------------    

/// Agent that tries to match the sub-pattern under `terminus` to any element
/// within some container of nodes. 
class SearchContainerAgent : public virtual PreRestrictedAgent
{
private:
    TreePtr<Node> terminus_key;

public:
    class TerminusMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                  
    virtual bool ImplHasNLQ() const;
    virtual void KeyReplace( const CouplingKeysMap *coupling_keys ); 
    virtual TreePtr<Node> BuildReplaceImpl();
    virtual shared_ptr<ContainerInterface> GetContainerInterface( XLink base_xlink ) const = 0;
    virtual XLink GetXLinkFromIterator( XLink base_xlink, ContainerInterface::iterator it ) const = 0;
    virtual void PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const {};
    virtual void DecidedQueryRestrictions( DecidedQueryAgentInterface &query, ContainerInterface::iterator thistime, XLink base_xlink ) const {};
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;

    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
    virtual const TreePtrInterface *GetTerminus() const
    {
        return &terminus;
    }
};

//---------------------------------- AnyNode ------------------------------------    
// TODO really badly named. How about Child?
/// Agent that matches the sub-pattern under `terminus` to any child of the
/// current node. This includes children in `Collection`s. It can be used to 
/// move "one level" down a tree with no assumption as to the nature of the node
/// being bypassed.
class AnyNodeAgent : public SearchContainerAgent
{
    class NoParentMismatch : public Mismatch {};
    virtual shared_ptr<ContainerInterface> GetContainerInterface( XLink base_xlink ) const;
    virtual XLink GetXLinkFromIterator( XLink base_xlink, ContainerInterface::iterator it ) const;                                             
    virtual void RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
};


/// Agent that matches any single node, with terminus support
template<class PRE_RESTRICTION>
class AnyNode : public AnyNodeAgent, 
                public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }    
};

//---------------------------------- Stuff ------------------------------------    

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
class StuffAgent : public SearchContainerAgent
{
public:
    virtual shared_ptr<ContainerInterface> GetContainerInterface( XLink base_xlink ) const;
    virtual XLink GetXLinkFromIterator( XLink base_xlink, ContainerInterface::iterator it ) const;
    virtual void PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const;
    virtual void DecidedQueryRestrictions( DecidedQueryAgentInterface &query, ContainerInterface::iterator thistime, XLink base_xlink ) const;
    virtual void RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;

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
};

};

#endif
