#ifndef SEARCH_CONTAINER_AGENT_HPP
#define SEARCH_CONTAINER_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

/// Agent that can match a terminus within a supplied container of nodes
class SearchContainerAgent : public virtual AgentCommon
{
    /// A kind of couplings key that allows a found terminus to be remembered    
    class TerminusKey : public Key // TODO put in SearchContainerAgent
    {
    public:
        TreePtr<Node> terminus;
    };

public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x ) = 0;

    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
};


/// Agent that matches an arbitrary subtree, with restrictions on elements therein and terminus support 
class StuffAgent : public SearchContainerAgent
{
public:
    StuffAgent();
    void AgentConfigure( const CompareReplace *s, CouplingKeys *c );
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x );

    TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
    CompareReplace * recurse_comparer;
};

/// Agent that matches an arbitrary subtree, with restrictions on elements therein and terminus support 
template<class PRE_RESTRICTION>
class Stuff : public StuffAgent, 
              public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS
};

/// Agent that matches any single node, with terminus support
class AnyNodeAgent : public SearchContainerAgent
{
public:
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x );
};


/// Agent that matches any single node, with terminus support
template<class PRE_RESTRICTION>
class AnyNode : public AnyNodeAgent, 
                public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS
};

};

#endif