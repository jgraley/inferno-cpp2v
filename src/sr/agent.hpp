#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "conjecture.hpp"
#include "boolean_evaluator.hpp"
#include <vector>
#include <boost/type_traits.hpp>
  
namespace SR
{ 
class SpecialBase;
class Engine;
class Agent;

class PatternQueryResult 
{
public:    
    struct Block 
    {
        bool abnormal;
        Agent *agent;
    };
        
    void clear()
    {
		blocks.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
    void AddLink( bool abnormal, Agent *a );
    void AddEvaluator( shared_ptr<BooleanEvaluator> e );
    
    list<Block> GetBlocks() const { return blocks; }
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }

private:
    list<Block> blocks;
    shared_ptr<BooleanEvaluator> evaluator;
};


class DecidedQueryResult 
{
public:    
    struct Block 
    {
        const TreePtrInterface *GetPX() const
        {
			if( px )
				return px; // linked x is in input tree
			else
				return &local_x; // linked x is local, kept alive by local_x    
		}	

        bool is_link;
        bool abnormal;
        Agent *agent;
        const TreePtrInterface *px;
        TreePtr<Node> local_x;
        bool is_decision;
        Conjecture::Range decision;
    };
        
    void clear()
    {
        blocks.clear();
        decision_count = 0;
        evaluator = shared_ptr<BooleanEvaluator>();        
    }

    void AddLink( bool abnormal, Agent *a, const TreePtrInterface *px ); 
    void AddLocalLink( bool abnormal, Agent *a, TreePtr<Node> x ); 
    void AddEvaluator( shared_ptr<BooleanEvaluator> e ); 
    ContainerInterface::iterator AddDecision( ContainerInterface::iterator begin,
                                              ContainerInterface::iterator end,
                                              bool inclusive,
                                              const Conjecture::Choices &choices,
                                              std::shared_ptr<ContainerInterface> container=nullptr );
    void AddLocalMatch( bool local_match );
                                                  
    list<Block> GetBlocks() const { return blocks; }
    shared_ptr<BooleanEvaluator> GetEvaluator() const { return evaluator; }
    bool IsLocalMatch() { return local_match; }
    int GetDecisionCount() const { return decision_count; }
    
private:
    list<Block> blocks; 
    shared_ptr<BooleanEvaluator> evaluator;
    bool local_match = true;
    int decision_count = 0;
};

bool operator<(const DecidedQueryResult::Block &l0, const DecidedQueryResult::Block &l1);


/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    // C++11 fix
    Agent& operator=(Agent& other)
    {
        (void)Node::operator=(other);
        return *this;
    }

    /// List the Agents reached via blocks during search
    virtual PatternQueryResult PatternQuery() const = 0;
    /// Produce info about an Agent given location (x) and a vector of choices (conj). 
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const Conjecture::Choices &choices,
                                             const Conjecture::Ranges &decisions ) const { return DecidedQuery(px, choices); }                                
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const Conjecture::Choices &choices ) const { ASSERT(!"implement DecidedQuery()"); return DecidedQueryResult(); }                                
    virtual TreePtr<Node> GetCoupled() = 0;                                  
    virtual void ResetKey() = 0;     
    virtual void KeyReplace( const TreePtrInterface &x,
                             Conjecture::Choices choices ) = 0;
    virtual void TrackingKey( Agent *from ) = 0;
    virtual TreePtr<Node> BuildReplace() = 0;
	virtual void AgentConfigure( const Engine *e ) = 0;
    virtual shared_ptr<ContainerInterface> GetVisibleChildren() const = 0;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const = 0;
		
	static Agent *AsAgent( TreePtr<Node> node );
};


// - Configure
// - Pre-restriction
// - Keying (default case)
// - Compare and MAtchingDecidedCompare rings
/// Implments misc functionality common to all or most agents
class AgentCommon : public Agent
{
public:
    AgentCommon();
    void AgentConfigure( const Engine *e );
    virtual shared_ptr<ContainerInterface> GetVisibleChildren() const;
    void DoKey( TreePtr<Node> x );
    TreePtr<Node> GetCoupled();                                  
    void ResetKey();    
    virtual void KeyReplace( const TreePtrInterface &x,
                             Conjecture::Choices choices );
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplace();
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
protected:
    const Engine *engine;    
			
private:    
    TreePtr<Node> coupling_key;    
};


// --- General note on SPECIAL_NODE_FUNCTIONS and PRE_RESTRICTION ---
// Special nodes (that is nodes defined here with special S&R behaviour)
// derive from a standard tree node via templating. This base class is
// the PRE_RESTRICTION node, and we want it for 2 reasons:
// 1. To allow standard nodes to point to special nodes, they must
//    expose a standard interface, which can vary depending on usage
//    so must be templated.
// 2. We are able to provide a "free" and-rule restriction on all
//    special nodes by restricting to non-strict subclasses of the
//    pre-restrictor.
// In order to make 2. work, we need to *avoid* overriding IsLocalMatch()
// or IsSubclass() on special nodes, so that the behaviour of the 
// PRE_RESTRICTION is preserved wrt comparisons. So all special nodes
// including speicialisations of TransformTo etc should use 
// SPECIAL_NODE_FUNCTIONS instead of NODE_FUNCTIONS.
// Itemise is known required (for eg graph plotting), other bounces
// are TBD.
#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  
/// Common stuff for pattern nodes other than standard nodes
class SpecialBase
{
public:    
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype() = 0;
};


/// Common stuff for pattern nodes other than standard nodes
template<class PRE_RESTRICTION>
class Special : public SpecialBase, 
                public virtual PRE_RESTRICTION
{
public:
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype()
    {
        // Esta muchos indirection
        return shared_ptr<TreePtrInterface>( new TreePtr<PRE_RESTRICTION>( new PRE_RESTRICTION ));  
    }
};

typedef Map< Agent *, TreePtr<Node> > CouplingMap;
};
#endif
