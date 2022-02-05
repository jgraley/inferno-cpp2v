#ifndef STANDARD_AGENT_HPP
#define STANDARD_AGENT_HPP

#include "agent_intermediates.hpp"
#include "agent_common.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include <set>
#include <boost/type_traits.hpp>
 
namespace SR
{ 

/// Agent implementation used for all standard pattern nodes. Normal agents are
/// constructed using `MakePatternPtr`. All the `TreePtr` members are treated
/// as pointers to sub-patterns and must match. Direct members (i.e. not in a 
/// `Collection`) may be nullptr to indicate a full wildcard (matches anything). 
class StandardAgent : public virtual PreRestrictedAgent
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
    
	virtual void SCRConfigure( const SCREngine *e,
                               Phase phase );

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
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink keyer_xlink ) const;                  
    void DecidedQuerySequence( DecidedQueryAgentInterface &query,
                               XLink keyer_xlink,
                               SequenceInterface *x_seq,
    	                       const Plan::Sequence &plan_seq ) const;
    void DecidedQueryCollection( DecidedQueryAgentInterface &query,
                                 XLink keyer_xlink,
                                 CollectionInterface *x_col,
    		                     const Plan::Collection &plan_col ) const;
    void DecidedQuerySingular( DecidedQueryAgentInterface &query,
                               XLink keyer_xlink,
                               TreePtrInterface *p_x_singular,
	                           const Plan::Singular &plan_sing ) const;
                                           
    virtual bool ImplHasSNLQ() const;                                            
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryPRed() const;                                       
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuerySequence(const Plan::Sequence &plan_seq) const;                                       
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryCollection(const Plan::Collection &plan_col) const;                                       
    SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuerySingular(const Plan::Singular &plan_sing) const;                                       
                                    
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;                                              
    void RegenerationQuerySequence( DecidedQueryAgentInterface &query,
                                    SequenceInterface *x_seq,
                                    const Plan::Sequence &plan_seq,
                                    const SolutionMap *hypothesis_links,
                                    const TheKnowledge *knowledge ) const;
    void RegenerationQueryCollection( DecidedQueryAgentInterface &query,
                                      CollectionInterface *x_col,
                                      const Plan::Collection &plan_col,
                                      const SolutionMap *hypothesis_links,
                                      const TheKnowledge *knowledge ) const;
                                           
public:
    virtual void PlanOverlayImpl(      PatternLink me_plink, 
                                    PatternLink under_plink );
    virtual TreePtr<Node> BuildReplaceImpl( PatternLink me_plink, 
                                            TreePtr<Node> key_node );

private:	
	TreePtr<Node> BuildReplaceOverlay( PatternLink me_plink, 
                                       TreePtr<Node> keynode ); // under substitution if not nullptr
    TreePtr<Node> BuildReplaceNormal( PatternLink me_plink );
    
public:
    virtual Block GetGraphBlockInfo() const;

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

	template<typename CP0>
    StandardAgentWrapper(const CP0 &cp0) : 
        NODE_TYPE(cp0) {}

	template<typename CP0, typename CP1>
    StandardAgentWrapper(const CP0 &cp0, const CP1 &cp1) : 
        NODE_TYPE(cp0, cp1) {}

    template<typename CP0, typename CP1, typename CP2>
    StandardAgentWrapper(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : 
        NODE_TYPE(cp0, cp1, cp2) {}
    
    template<typename CP0, typename CP1, typename CP2, typename CP3>
    StandardAgentWrapper(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2, const CP2 &cp3) : 
        NODE_TYPE(cp0, cp1, cp2, cp3) {}
    
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
        return TreePtr<Node>( new NODE_TYPE );  
    }
       
    // disambiguate between Agent and Node: Agent wins
    virtual string GetTrace() const override
    {
        return StandardAgent::GetTrace();
    }
};


/// Helper that does the constructing of pattern nodes when they are not already agents
template< bool IS_AGENT, typename NODE_TYPE >
class MakePatternPtrHelper  
{
public:    
    template<typename ... CP>
    static inline TreePtr<NODE_TYPE> Make(const CP& ... cp)
    {
        return TreePtr<NODE_TYPE>(new StandardAgentWrapper<NODE_TYPE>(cp...));
    }    
};


/// Helper that does the constructing of pattern nodes that are already agents
template<typename NODE_TYPE>
class MakePatternPtrHelper<true, NODE_TYPE> // NODE_TYPE is an agent, so behave like MakeTreePtr
{
public:
    template<typename ... CP>
    static inline TreePtr<NODE_TYPE> Make(const CP&... cp)
    {
        return TreePtr<NODE_TYPE>(new NODE_TYPE(cp...));
    }    
};


// Similar to MakeTreePtr<> (see node/specialise_oostd.hpp) but produces a TreePtr to StandardAgentWrapper<NODE_TYPE> rather
// than just NODE_TYPE when NODE_TYPE is not already a kind of Agent. 
/// Utility for constructing nodes that are to be used in patterns from standard tree nodes
template<typename NODE_TYPE>
class MakePatternPtr : public TreePtr<NODE_TYPE>
{
private:
    // Using the magic of Boost, find out at compile time whether the NODE_TYPE is already an Agent.	
	typedef MakePatternPtrHelper<is_base_of<Agent, NODE_TYPE>::value, NODE_TYPE> Maker;
public:	
	template<typename ... CP>
	inline MakePatternPtr(const CP& ... cp) : 
	    TreePtr<NODE_TYPE>( Maker::Make(cp...) ) 
	{ 
        //TRACE("MakePatternPtr made a ")(*this)("\n");
	}
};

};

#endif
