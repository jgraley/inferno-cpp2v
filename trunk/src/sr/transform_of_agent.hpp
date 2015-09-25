#ifndef TRANSFORM_OF_AGENT_HPP
#define TRANSFORM_OF_AGENT_HPP

#include "search_replace.hpp"
#include "helpers/transformation.hpp"
#include "agent.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"

namespace SR
{
/// Match the output of some transformation against the child pattern 
class TransformOfAgent : public virtual AgentCommon
{
public:
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape );
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
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfAgent(t, p) 
    {
    }
};
};
#endif
