#ifndef STAR_AGENT_HPP
#define STAR_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Agent used in a container pattern under a standard agent, that matches 
/// zero or more elements
/// Star can match more than one node of any type in a container
/// In a Sequence, only a contiguous subsequence of 0 or more elements will match
/// In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
/// Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
/// in the collection.
/// `restriction` may point to a sub-pattern that must match all nodes 
/// within the subcollection matched by the star node. Some limitations exist regarding
/// the limitations of the use of restrictions in mutliple star nodes. TODO: clarify.
/// The restriction is an abnormal context because it can match zero or more 
/// different subtrees.
class StarAgent : public virtual AgentCommon 
{
public:
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      TreePtr<Node> x ) const;                  
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
private:
    virtual const TreePtrInterface *GetRestriction() const = 0;

public:
};


/// Agent used in a container pattern that matches zero or more elements
template<class PRE_RESTRICTION>
class Star : public StarAgent, 
             public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> restriction; 
    virtual const TreePtrInterface *GetRestriction() const
    {
        return &restriction;
    }
};

};

#endif