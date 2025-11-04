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
/// Utility for constructing nodes that are to be used in patterns from standard tree nodes
template<typename NODE_TYPE, typename ... CP>
TreePtr<NODE_TYPE> MakePatternNode(const CP &...cp)
{
    // Find out at compile time whether the NODE_TYPE is already an Agent.    
    return MakePatternNodeHelper<is_base_of<Agent, NODE_TYPE>::value, NODE_TYPE>::MakeNode(cp...); 
};


// Similar to MakeTreeNode<> (see node/tree_ptr.hpp) but produces a TreePtr to 
// StandardAgentWrapper<NODE_TYPE> rather than just NODE_TYPE when NODE_TYPE 
// is not already a kind of Agent. 
/// Utility for constructing nodes that are to be used in patterns from standard tree nodes
template<typename AGENT_TYPE, typename PRE_RESTRICTION, typename ... CP>
TreePtr<AGENT_TYPE> MakePatternNode(const CP &...cp)
{
    // Find out at compile time whether the NODE_TYPE is already an Agent.    
    TreePtr<AGENT_TYPE> agent_node = MakeTreeNode<AGENT_TYPE>(cp...); 
    agent_node->pre_restriction_archetype_node = shared_ptr<Node>( new PRE_RESTRICTION );
    agent_node->pre_restriction_archetype_ptr = make_shared<TreePtr<PRE_RESTRICTION>>();
    return agent_node;    
};

 

/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<class PRE_RESTRICTION>
class EmbeddedSCR : public EmbeddedSCRAgent, 
                    public Special<PRE_RESTRICTION>
{
public:
    
    // EmbeddedSearchReplace must be constructed using constructor
    EmbeddedSCR( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
        EmbeddedSCRAgent( sp, rp, is_search )
    {
		through = t;
    }
};


/// EmbeddedSCR that performs a seperate compare and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class EmbeddedCompareReplace : public EmbeddedSCR<PRE_RESTRICTION>
{
public:
    EmbeddedCompareReplace() : EmbeddedSCR<PRE_RESTRICTION>( nullptr, nullptr, nullptr, false ) {}      
    EmbeddedCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCR<PRE_RESTRICTION>( t, sp, rp, false ) {}

};


/// EmbeddedSCR that performs a seperate search and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class EmbeddedSearchReplace : public EmbeddedSCR<PRE_RESTRICTION>
{
public:
    EmbeddedSearchReplace() : EmbeddedSCR<PRE_RESTRICTION>( nullptr, nullptr, nullptr, true ) {}      
    EmbeddedSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        EmbeddedSCR<PRE_RESTRICTION>( t, sp, rp, true ) {}
};


// ---------------------- TransformOf<> ---------------------------

/// Match the output of some transformation against the child pattern 
template<class PRE_RESTRICTION>
class TransformOf : public TransformOfAgent,
                    public Special<PRE_RESTRICTION>
{
public:   
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfAgent(t, p) 
    {
    }
};


/// Agent used in a container pattern that matches zero or more elements
template<class PRE_RESTRICTION>
class Star : public StarAgent, 
             public Special<PRE_RESTRICTION>
{
};


template<class PRE_RESTRICTION>
class PointerIs : public Special<PRE_RESTRICTION>,
                  public PointerIsAgent
{
};


template<class PRE_RESTRICTION>
class Negation : public Special<PRE_RESTRICTION>,
                 public NegationAgent
{
};
  

template<class PRE_RESTRICTION>
class GreenGrass : public GreenGrassAgent, 
                   public Special<PRE_RESTRICTION>
{
};


template<class PRE_RESTRICTION>
class Disjunction : public Special<PRE_RESTRICTION>,
                    public DisjunctionAgent
{
};


/// Agent that matches any single node, with terminus support
template<class PRE_RESTRICTION>
class Child : public ChildAgent, 
                public Special<PRE_RESTRICTION> 
{
};


template<class PRE_RESTRICTION>
class Stuff : public StuffAgent, 
              public Special<PRE_RESTRICTION> 
{
};


template<class PRE_RESTRICTION>
class Delta : public DeltaAgent, 
              public Special<PRE_RESTRICTION>
{
};


template<class PRE_RESTRICTION>
class Conjunction : public Special<PRE_RESTRICTION>,
                    public ConjunctionAgent
{
};

}; // end namespace

#endif

