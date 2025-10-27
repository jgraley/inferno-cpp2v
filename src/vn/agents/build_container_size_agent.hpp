#ifndef BUILD_CONTAINER_SIZE_AGENT_HPP
#define BUILD_CONTAINER_SIZE_AGENT_HPP

#include "vn/agents/relocating_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/star_agent.hpp"
#include "vn/agents/builder_agent.hpp"
#include "vn/sym/expression.hpp"

using namespace VN;

/// `BuildContainerSizeAgent` is used in replace context to create an integer-valued
/// constant that is the size of a `Star` node pointed to by `container`. The
/// container should couple the star node.
struct BuildContainerSizeAgent : public virtual BuilderAgent,
                                 Special<CPPTree::Integer>
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TreePtr<Node> container;
private:
    virtual TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) override;
    
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final; 
    bool IsFixedType() const final;           
    NodeBlock GetGraphBlockInfo() const final;
}; 

#endif
