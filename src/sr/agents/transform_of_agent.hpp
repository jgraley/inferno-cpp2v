#ifndef TRANSFORM_OF_AGENT_HPP
#define TRANSFORM_OF_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "agent.hpp"
#include "delta_agent.hpp"
#include "slave_agent.hpp"
#include "../cache.hpp"

namespace SR
{
/// Matches the output of `transformation` when applied to the current tree node
/// against the sub-pattern at `pattern`. This permits an arbitrary `Transformation`
/// implementation to be "injected" into the search and replace operation.
/// The transformation should be invarient during any single search operation
/// but can change when replace acts to change the tree. Thus it can depend on
/// the tree. The overall root of the tree is supplied as context to the 
/// transformation, even when processing slaves.
class TransformOfAgent : public virtual TeleportAgent
{
public:
    class TransformationFailedMismatch : public Mismatch {};

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual map<PatternLink, XLink> RunTeleportQuery( XLink base_xlink ) const;                
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
    TreePtr<Node> pattern; 
    Transformation *transformation;
    TransformOfAgent( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

protected: 
    TransformOfAgent() {}    
};


/// Match the output of some transformation against the child pattern 
template<class PRE_RESTRICTION>
class TransformOf : public TransformOfAgent,
                    public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS	

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfAgent(t, p) 
    {
    }
};
};
#endif
