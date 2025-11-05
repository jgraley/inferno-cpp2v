#ifndef PATTERN_HELPERS_HPP
#define PATTERN_HELPERS_HPP

#include "tree/cpptree.hpp"
#include "tree/sctree.hpp"
#include "common/common.hpp"
#include "vn/agents/all.hpp"
#include "tree/typeof.hpp"
#include "steps/uncombable.hpp"

namespace Steps {


/// Helper that does the constructing of pattern nodes when they are not already agents
template< bool IS_AGENT, typename NODE_TYPE >
class MakePatternNodeHelper  
{
public:    
    template<typename ... CP>
    static inline TreePtr<NODE_TYPE> MakeNode(const CP &...cp)
    {
        auto agent_node = MakeTreeNode<StandardAgentWrapper<NODE_TYPE>>(cp...);
        agent_node->my_archetype_node = shared_ptr<Node>( new NODE_TYPE );
        agent_node->my_archetype_ptr = make_shared<TreePtr<NODE_TYPE>>();
        return agent_node;        
    }    
};


/// Helper that does the constructing of pattern nodes that are already agents
template<typename AGENT_TYPE>
class MakePatternNodeHelper<true, AGENT_TYPE> // NODE_TYPE is an agent, so behave like MakeTreeNode
{
public:
	typedef AGENT_TYPE::PreRestrictionType PreRestrictionType;

    template<typename ... CP>
    static inline TreePtr<AGENT_TYPE> MakeNode(const CP &...cp)
    {
        auto agent_node = MakeTreeNode<AGENT_TYPE>(cp...);
        // These members are introduced by Special
        agent_node->pre_restriction_archetype_node = shared_ptr<Node>( new PreRestrictionType );
        agent_node->pre_restriction_archetype_ptr = make_shared<TreePtr<PreRestrictionType>>();
        return agent_node;
    }    
};


// Similar to MakeTreeNode<> (see node/tree_ptr.hpp) but produces a TreePtr to 
// StandardAgentWrapper<NODE_TYPE> rather than just NODE_TYPE when NODE_TYPE 
// is not already a kind of Agent. 
template<typename NODE_TYPE, typename ... CP>
TreePtr<NODE_TYPE> MakePatternNode(const CP &...cp)
{
    // Find out at compile time whether the NODE_TYPE is already an Agent.    
    return MakePatternNodeHelper<is_base_of<Agent, NODE_TYPE>::value, NODE_TYPE>::MakeNode(cp...); 
};


// 2-argument version of MakePatternNode<>() for pre-restrictable agents
// which takes agent and pre-res types separately
template<typename AGENT_TYPE, typename PRE_RESTRICTION, typename ... CP>
TreePtr<AGENT_TYPE> MakePatternNode(const CP &...cp)
{
    // Find out at compile time whether the NODE_TYPE is already an Agent.    
    TreePtr<AGENT_TYPE> agent_node = MakeTreeNode<AGENT_TYPE>(cp...); 
    agent_node->pre_restriction_archetype_node = shared_ptr<Node>( new PRE_RESTRICTION );
    agent_node->pre_restriction_archetype_ptr = make_shared<TreePtr<PRE_RESTRICTION>>();
    return agent_node;    
};

// For pre-restrictable agents but no pre-res applied
template<typename AGENT_TYPE, typename ... CP>
TreePtr<AGENT_TYPE> MakePatternNodeNP(const CP &...cp)
{
    // Find out at compile time whether the NODE_TYPE is already an Agent.    
    return MakeTreeNode<AGENT_TYPE>(cp...);     
};


/// EmbeddedSCR that performs a seperate compare and replace operation at the corresponding place in the output tree
class EmbeddedCompareReplaceAgent : public EmbeddedSCRAgent
{
public:
    EmbeddedCompareReplaceAgent() : EmbeddedSCRAgent( nullptr, nullptr, false ) {}      
    EmbeddedCompareReplaceAgent( TreePtr<Node> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCRAgent( sp, rp, false )
    {
		through = t;
    }
};


/// EmbeddedSCR that performs a seperate search and replace operation at the corresponding place in the output tree
class EmbeddedSearchReplaceAgent : public EmbeddedSCRAgent
{
public:
    EmbeddedSearchReplaceAgent() : EmbeddedSCRAgent( nullptr, nullptr, true ) {}      
    EmbeddedSearchReplaceAgent( TreePtr<Node> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCRAgent( sp, rp, true )
    {
		through = t;
    }
};

}; // end namespace

#endif

