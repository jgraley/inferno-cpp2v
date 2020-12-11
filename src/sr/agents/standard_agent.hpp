#ifndef STANDARD_AGENT_HPP
#define STANDARD_AGENT_HPP

#include "agent.hpp"
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
class StandardAgent : public virtual AgentCommon
{
public:
    class Mismatch : public ::Mismatch {};
    
    class SingularMismatch : public Mismatch {};
    
    class SequenceMismatch : public Mismatch {};
    class WrongContainerSequenceMismatch : public SequenceMismatch {}; 
    class NotAfterSequenceMismatch : public SequenceMismatch {};
    class NotSuccessorSequenceMismatch : public SequenceMismatch {};
    class SurplusXSequenceMismatch : public SequenceMismatch {};
    
    class CollectionMismatch : public Mismatch {};
    class WrongContainerCollectionMismatch : public CollectionMismatch {};
    class CollisionCollectionMismatch : public CollectionMismatch {};
    class SurplusXCollectionMismatch : public CollectionMismatch {};
    
	virtual void AgentConfigure( const SCREngine *master_scr_engine );

    struct Plan
    {
        void ConstructPlan( StandardAgent *algo );
        void MakePatternQuery();
        
        struct Sequence
        {
            Sequence( Plan *plan, SequenceInterface *pattern );
            
            int num_non_star;
            int num_stars;
            ContainerInterface::iterator pit_last_star;
            set<PatternLink> stars_preceding_non_stars;
        };
        
        struct Collection
        {
            Collection( Plan *plan, CollectionInterface *pattern );
            
            const TreePtrInterface *p_star;
            PatternLink star_plink;
        };
        
        StandardAgent *algo;
        map<SequenceInterface *, Sequence> sequences;
        map<CollectionInterface *, Collection> collections;
        shared_ptr<PatternQuery> pattern_query;
    };
    
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                  
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;

private:
    void DecidedQuerySequence( DecidedQueryAgentInterface &query,
                               XLink base_xlink,
                               SequenceInterface *px,
    	                       SequenceInterface *pattern ) const;
    void DecidedQueryCollection( DecidedQueryAgentInterface &query,
                                 XLink base_xlink,
                                 CollectionInterface *px,
    		                     CollectionInterface *pattern ) const;
    virtual bool ImplHasDNLQ() const;
    virtual Completeness RunDecidedNormalLinkedQueryImpl( DecidedQueryAgentInterface &query,
                                                          XLink base_xlink,
                                                          const SolutionMap *required_links,
                                                          const TheKnowledge *knowledge ) const;                                              
    void DecidedNormalLinkedQuerySequence( DecidedQueryAgentInterface &query,
                                           XLink base_xlink,
                                           SequenceInterface *px,
                                           SequenceInterface *pattern,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge,
                                           Completeness &completeness ) const;
    void DecidedNormalLinkedQueryCollection( DecidedQueryAgentInterface &query,
                                             XLink base_xlink,
                                             CollectionInterface *px,
                                             CollectionInterface *pattern,
                                             const SolutionMap *required_links,
                                             const TheKnowledge *knowledge,
                                             Completeness &completeness ) const;
                                           
public:
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplaceImpl( CouplingKey keylink=CouplingKey() );

private:	
	TreePtr<Node> BuildReplaceOverlay( TreePtr<Node> keynode ); // under substitution if not nullptr
    TreePtr<Node> BuildReplaceNormal();
    
    Plan plan; // can't be const because children added after construct
    bool planned = false;
};


#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  

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
        // node type and use that.
        return string("StandardAgent<") + NODE_TYPE().GetTypeName() + string(">");        
    }

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
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
        return new StandardAgentWrapper<NODE_TYPE>(cp...);
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
        return new NODE_TYPE(cp...);
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
