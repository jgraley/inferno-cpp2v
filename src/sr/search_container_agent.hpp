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
class SearchContainerAgent : public virtual AgentCommon
{
private:
    TreePtr<Node> terminus_key;

public:
    virtual PatternQueryResult PatternQuery() const;
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const;
    virtual void KeyReplace( const TreePtrInterface &x,
                             AgentQuery::Choices choices ); 
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x ) const = 0;
    virtual void PatternQueryRestrictions() const {};
    virtual void DecidedQueryRestrictions( DecidedQueryResult &r, ContainerInterface::iterator thistime ) const {};

    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
};

//---------------------------------- AnyNode ------------------------------------    
// TODO really badly named. How about Child?
/// Agent that matches the sub-pattern under `terminus` to any child of the
/// current node. This includes children in `Collection`s. It can be used to 
/// move "one level" down a tree with no assumption as to the nature of the node
/// being bypassed.
class AnyNodeAgent : public SearchContainerAgent
{
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
};


/// Agent that matches any single node, with terminus support
template<class PRE_RESTRICTION>
class AnyNode : public AnyNodeAgent, 
                public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS
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
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x ) const;
    virtual void PatternQueryRestrictions( PatternQueryResult &r ) const;
    virtual void DecidedQueryRestrictions( DecidedQueryResult &r, ContainerInterface::iterator thistime ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
};


template<class PRE_RESTRICTION>
class Stuff : public StuffAgent, 
              public Special<PRE_RESTRICTION> 
{
public:
    SPECIAL_NODE_FUNCTIONS
};

};

#endif
