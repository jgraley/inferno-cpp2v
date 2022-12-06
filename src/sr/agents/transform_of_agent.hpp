#ifndef TRANSFORM_OF_AGENT_HPP
#define TRANSFORM_OF_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "teleport_agent.hpp"
#include "delta_agent.hpp"
#include "embedded_scr_agent.hpp"
#include "special_agent.hpp"

namespace SR
{
/// Matches the output of `transformation` when applied to the current tree node
/// against the sub-pattern at `pattern`. This permits an arbitrary `Transformation`
/// implementation to be "injected" into the search and replace operation.
/// The transformation should be invarient during any single search operation
/// but can change when replace acts to change the tree. Thus it can depend on
/// the tree.
class TransformOfAgent : public virtual TeleportAgent
{
public:
    class TransformationFailedMismatch : public Mismatch {};

    TransformOfAgent( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    TeleportResult RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink start_xlink ) const override;                

    virtual Block GetGraphBlockInfo() const;
    string GetName() const override;
    
 	bool IsExtenderLess( const Extender &r ) const override;
	int GetExtenderOrdinal() const override;

    TreePtr<Node> pattern; 
    Transformation *transformation;

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
