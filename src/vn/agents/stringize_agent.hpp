#ifndef STRINGIZE_AGENT_HPP
#define STRINGIZE_AGENT_HPP

#include "vn/agents/relocating_agent.hpp"
#include "tree/cpptree.hpp" 
#include "helpers/transformation.hpp"
#include "tree/sctree.hpp"
#include "vn/agents/star_agent.hpp"
#include "vn/agents/builder_agent.hpp"
#include "vn/sym/expression.hpp"

using namespace VN;

// Make a String node containing the token string of an identifier. No uniquing is attempted.
struct StringizeAgent : Special<CPPTree::String>,
                        public virtual BuilderAgent
{
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }

    StringizeAgent() {}

    TreePtr<CPPTree::Identifier> source;

private:
    TreePtr<Node> BuildNewSubtree(const SCREngine *acting_engine) final;    
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    bool IsNonTrivialPreRestriction(const TreePtrInterface *pptr) const final;    
    NodeBlock GetGraphBlockInfo() const override;
};

#endif
