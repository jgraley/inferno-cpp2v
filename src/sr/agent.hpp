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

class PatternLinks
{
public:    
    struct Link 
    {
        bool abnormal;
        Agent *agent;
    };
        
    vector<Link> links;
    shared_ptr<BooleanEvaluator> evaluator;

    void clear()
    {
		links.clear();
        evaluator = shared_ptr<BooleanEvaluator>();
    }
};


class Links 
{
public:    
    struct Link : PatternLinks::Link
    {
        const TreePtrInterface *px;
        TreePtr<Node> local_x;
        const TreePtrInterface *GetX() const
        {
			if( px )
				return px; // linked pattern is in input tree
			else
				return &local_x; // linked pattern is local, kept alive by local_x    
		}	
    };
        
    vector<Link> links; 
    deque<Conjecture::Range> decisions;
    shared_ptr<BooleanEvaluator> evaluator;
    bool local_match;

    void clear()
    {
        links.clear();
        decisions.clear();
        evaluator = shared_ptr<BooleanEvaluator>();        
    }
};

bool operator<(const Links::Link &l0, const Links::Link &l1);


/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    /// List the Agents reached via normal links during search
    virtual PatternLinks PatternQuery() const = 0;
    /// Produce info about an Agent given location (x) and a vector of choices (conj). 
    virtual Links DecidedQuery( const TreePtrInterface &x,
                                deque<ContainerInterface::iterator> choices ) const = 0;                                
    virtual TreePtr<Node> GetCoupled() = 0;                                  
    virtual void ResetKey() = 0;     
    virtual void KeyReplace( const TreePtrInterface &x,
                             deque<ContainerInterface::iterator> choices ) = 0;
    virtual void TrackingKey( Agent *from ) = 0;
    virtual TreePtr<Node> BuildReplace() = 0;
	virtual void AgentConfigure( const Engine *e ) = 0;
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
    virtual PatternLinks PatternQuery() const;
    virtual void PatternQueryImpl() const = 0;
    virtual Links DecidedQuery( const TreePtrInterface &x,
                                deque<ContainerInterface::iterator> choices ) const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const = 0;
    void DoKey( TreePtr<Node> x );
    TreePtr<Node> GetCoupled();                                  
    void ResetKey();    
    virtual void KeyReplace( const TreePtrInterface &x,
                             deque<ContainerInterface::iterator> choices );
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplace();
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
protected:
    void RememberLink( bool abnormal, Agent *a ) const; // Pattern query
    void RememberLink( bool abnormal, Agent *a, const TreePtrInterface &x ) const; // Decided query
    void RememberLocalLink( bool abnormal, Agent *a, TreePtr<Node> x ) const; // Decided query
    void RememberEvaluator( shared_ptr<BooleanEvaluator> e ) const; // All queries

    const Engine *engine;    
    ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
                                                 ContainerInterface::iterator end ) const;
    ContainerInterface::iterator RememberDecisionLink( bool abnormal, 
													   Agent *a, 
													   ContainerInterface::iterator begin,
													   ContainerInterface::iterator end ) const; // Decided query
			
private:    
    TreePtr<Node> coupling_key;    
    
    // These are mutable because they are the only things that change during 
    // a call to DecidedQuery(). It is "OK" because their usage lifetime is 
    // restricted to a single call (they do not carry state from one call to
    // the next). TODO pass in and out of DecidedQueryImpl() as params - some
    // restructuing needed to make this nice as there are modifiers here that
    // would need to operate on some new object that combines the "links" and
    // "choices".
    mutable enum {IDLE, PATTERN, DECIDED} current_query;
    mutable PatternLinks pattern_links;    
    mutable Links links;    
    mutable deque<ContainerInterface::iterator> choices;
};


// --- General note on SPECIAL_NODE_FUNCTIONS and PRE_RESTRICTION ---
// Special nodes (that is nodes defined here with special S&R behaviour)
// derive from a normal tree node via templating. This base class is
// the PRE_RESTRICTION node, and we want it for 2 reasons:
// 1. To allow normal nodes to point to special nodes, they must
//    expose a normal interface, which can vary depending on usage
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
/// Common stuff for pattern nodes other than normal nodes
class SpecialBase
{
public:    
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype() = 0;
};


/// Common stuff for pattern nodes other than normal nodes
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
