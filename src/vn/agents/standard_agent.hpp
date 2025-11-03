#ifndef STANDARD_AGENT_HPP
#define STANDARD_AGENT_HPP

#include "agent_intermediates.hpp"
#include "agent_common.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include <set>
 
namespace VN
{ 

/// Agent implementation used for all standard pattern nodes. Normal agents are
/// constructed using `MakePatternNode`. All the `TreePtr` members are treated
/// as pointers to sub-patterns and must match. Direct members (i.e. not in a 
/// `Collection`) may be nullptr to indicate a full wildcard (matches anything). 
class StandardAgent : public virtual ColocatingAgent
{
public:
    class Mismatch : public ::Mismatch {};
    
    class SingularMismatch : public Mismatch {};
    
    class SequenceMismatch : public Mismatch {};
    class WrongCadenceSequenceMismatch : public SequenceMismatch {}; 
    class WrongContainerSequenceMismatch : public SequenceMismatch {}; 
    class NotAtFrontMismatch : public SequenceMismatch {};
    class NotAtBackMismatch : public SequenceMismatch {};
    class NotAfterSequenceMismatch : public SequenceMismatch {};
    class NotSuccessorSequenceMismatch : public SequenceMismatch {};
    
    class CollectionMismatch : public Mismatch {};
    class WrongCadenceCollectionMismatch : public CollectionMismatch {};
    class WrongContainerCollectionMismatch : public CollectionMismatch {};
    class CollisionCollectionMismatch : public CollectionMismatch {};
    class SurplusXCollectionMismatch : public CollectionMismatch {};
    class InsufficientXCollectionMismatch : public CollectionMismatch {};
    
    virtual void SCRConfigure( Phase phase );

    struct Plan : public virtual Traceable
    {
        void ConstructPlan( StandardAgent *algo, Phase phase );
        string GetTrace() const; // used for debug

        struct Item
        {
            Item( int ii ) : 
                itemise_index(ii)
            {
            }
            int itemise_index;
        };
        
        struct Sequence : Item
        {
            Sequence( int ii, Plan *plan, Phase phase, SequenceInterface *pattern_ );
            
            struct Run
            {
                PatternLink predecessor; // nonstar before run or NULL of at start
                list<PatternLink> elts;  // run of stars, in order
                PatternLink successor;   // nonstar after run or NULL of at end
            };
            
            SequenceInterface *pattern;
            int num_non_star;
            ContainerInterface::iterator pit_last_star;
            PatternLink plink_front; // ?...
            PatternLink plink_back;  // ...?
            set<PatternLink> non_stars;    // ...?...        
            set< pair<PatternLink, PatternLink> > adjacent_non_stars; // ...??...
            set< pair<PatternLink, PatternLink> > gapped_non_stars; // ...?...?...
            PatternLink non_star_at_front; // ?...
            PatternLink non_star_at_back;  // ...?
            set< shared_ptr<Run> > star_runs; // ...?***?...
        };
        
        struct Collection : Item
        {
            Collection( int ii, Plan *plan, Phase phase, CollectionInterface *pattern_ );
            
            CollectionInterface *pattern;
            const TreePtrInterface *p_star;
            PatternLink star_plink;
            set<PatternLink> non_stars;
        };
        
        struct Singular : Item
        {
            Singular( int ii, Plan *plan, Phase phase, TreePtrInterface *pattern_ );
            
            TreePtrInterface *pattern;
            PatternLink plink;
        };
        
        StandardAgent *algo;
        list<Sequence> sequences;
        list<Collection> collections;
        list<Singular> singulars;
    };
    
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    void IncrPatternQuerySequence( const Plan::Sequence &plan_seq, 
                                   shared_ptr<PatternQuery> &pattern_query ) const;
    void IncrPatternQueryCollection( const Plan::Collection &plan_col, 
                                     shared_ptr<PatternQuery> &pattern_query ) const;
    void IncrPatternQuerySingular( const Plan::Singular &plan_sing, 
                                   shared_ptr<PatternQuery> &pattern_query ) const;

private:        
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed(PatternLink keyer_plink) const;                                       
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuerySequence(const Plan::Sequence &plan_seq, PatternLink keyer_plink) const;                                       
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQueryCollection(const Plan::Collection &plan_col, PatternLink keyer_plink) const;                                       
    SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuerySingular(const Plan::Singular &plan_sing, PatternLink keyer_plink) const;                                       
                                    
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           PatternLink keyer_plink,
                                           const XTreeDatabase *x_tree_db ) const;                                              
    void RegenerationQuerySequence( DecidedQueryAgentInterface &query,
                                    SequenceInterface *x_seq,
                                    const Plan::Sequence &plan_seq,
                                    const SolutionMap *hypothesis_links,
                                    PatternLink keyer_plink,
                                    const XTreeDatabase *x_tree_db ) const;
    void RegenerationQueryCollection( DecidedQueryAgentInterface &query,
                                      CollectionInterface *x_col,
                                      const Plan::Collection &plan_col,
                                      const SolutionMap *hypothesis_links,
                                      PatternLink keyer_plink,
                                      const XTreeDatabase *x_tree_db ) const;
                                           
public:
    virtual void MaybeChildrenPlanOverlay( SCREngine *acting_engine,
                                           PatternLink me_plink, 
                                           PatternLink bottom_layer_plink );
    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                    PatternLink me_plink, 
                                    XLink key_xlink,
                                                  const SCREngine *acting_engine ) final;

private:    
    ReplacePatchPtr GenReplaceLayoutOverlay( const ReplaceKit &kit, 
                                             PatternLink me_plink, 
                                             XLink under_xlink,
                                                  const SCREngine *acting_engine ); // under substitution if not nullptr
    ReplacePatchPtr GenReplaceLayoutOverlayUsingPattern( const ReplaceKit &kit, 
                                                         PatternLink me_plink, 
                                                         XLink under_xlink,
                                                  const SCREngine *acting_engine );
    ReplacePatchPtr GenReplaceLayoutOverlayUsingX( const ReplaceKit &kit, 
                                                   PatternLink me_plink, 
                                                   XLink under_xlink,
                                                  const SCREngine *acting_engine ); 
    ReplacePatchPtr GenReplaceLayoutNormal( const ReplaceKit &kit, 
                                            PatternLink me_plink,
                                                  const SCREngine *acting_engine );
    
public:
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    bool IsFixedType() const final;

    NodeBlock GetGraphBlockInfo() const final;

private:
    Plan plan; // can't be const because children added after construct
    bool planned = false;
};


/// Wrapper for normal pattern nodes that combines a StandardAgent with the specified pattern node
template<typename NODE_TYPE>
class StandardAgentWrapper : public virtual NODE_TYPE,
                             public virtual StandardAgent // please keep this ordering, so StandardAgent::StandardAgent can introspect the NODE_TYPE
{
public:
    StandardAgentWrapper() : NODE_TYPE() {}

    template<typename ... CP>
    StandardAgentWrapper(const CP &...cp) : 
        NODE_TYPE(cp...) {}
    
    virtual string GetTypeName() const // used by parse, render etc
    {
        // Want to get rid of the StandardAgentWrapper<...> around the name. The main
        // GetName() works via RTTI so no amount of casting of "this" will stop the full
        // final node's name being returned. So we have to actually make a temp in the
        // node type and use that. No, don't make a temp, it makes the serial numbers 
        // change depeding on when tracing is enabled. Just fiddle about with the string.
        string real_typename = Node::GetTypeName();        
        const string expected_prefix = "StandardAgentWrapper";
        const string desired_prefix = "StandardAgent";
        if( real_typename.rfind(expected_prefix, 0) == 0 )
            return desired_prefix + real_typename.substr(expected_prefix.length());
        else
            return real_typename; // hopefully contains something informative
    }

    shared_ptr<const Node> GetPatternPtr() const override
    {
        return shared_from_this();
    } 
    
    virtual TreePtr<Node> GetArchetypeNode() const override
    {
        return MakeTreeNode<NODE_TYPE>();  
    }
    
    shared_ptr< TreePtrInterface > GetArchetypeTreePtr() const override
    {
        return make_shared<TreePtr<NODE_TYPE>>();  
    }
       
    // disambiguate between Agent and Node: Agent wins
    virtual string GetTrace() const override
    {
        return StandardAgent::GetTrace();
    }
};


/// Helper that does the constructing of pattern nodes when they are not already agents
template< bool IS_AGENT, typename NODE_TYPE >
class MakePatternNodeHelper  
{
public:    
    template<typename ... CP>
    static inline TreePtr<NODE_TYPE> MakeNode(const CP &...cp)
    {
        return MakeTreeNode<StandardAgentWrapper<NODE_TYPE>>(cp...);
    }    
};


/// Helper that does the constructing of pattern nodes that are already agents
template<typename NODE_TYPE>
class MakePatternNodeHelper<true, NODE_TYPE> // NODE_TYPE is an agent, so behave like MakeTreeNode
{
public:
	typedef NODE_TYPE::PreRestrictionType PreRestrictionType;

    template<typename ... CP>
    static inline TreePtr<NODE_TYPE> MakeNode(const CP &...cp)
    {
        TreePtr<NODE_TYPE> agent_node = MakeTreeNode<NODE_TYPE>(cp...);
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
    TreePtr<AGENT_TYPE> agent_node =  MakePatternNodeHelper<is_base_of<Agent, AGENT_TYPE>::value, AGENT_TYPE>::MakeNode(cp...); 
    agent_node->pre_restriction_archetype_node = shared_ptr<Node>( new PRE_RESTRICTION );
    agent_node->pre_restriction_archetype_ptr = make_shared<TreePtr<PRE_RESTRICTION>>();
    return agent_node;    
};


};

#endif
