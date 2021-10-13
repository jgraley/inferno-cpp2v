#ifndef BUILDER_AGENT_HPP
#define BUILDER_AGENT_HPP

#include "../search_replace.hpp"
#include "agent.hpp"

namespace SR
{
    
struct BuilderAgent : public virtual SearchLeafAgent    
{
public:
	// TODO do this via a transformation as with TransformOf/TransformOf
    BuilderAgent() {}

    virtual TreePtr<Node> BuildNewSubtree() = 0;

private:
	virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> under_node ) override;  
};

};
#endif
