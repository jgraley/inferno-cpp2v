#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "conjecture.hpp"
#include "boolean_evaluator.hpp"
#include "query.hpp"
#include <vector>
#include <boost/type_traits.hpp>
  
namespace SR
{ 
class SpecialBase;
class Engine;
class Agent;

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
    virtual void DecidedQuery( const TreePtrInterface *px,
                               AgentQuery &state ) const { auto dqr = DecidedQuery(px, state.choices, state.decisions); state.SetDQR(dqr); }                                
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices,
                                             const AgentQuery::Ranges &decisions ) const { return DecidedQuery(px, choices); }                                
    virtual DecidedQueryResult DecidedQuery( const TreePtrInterface *px,
                                             const AgentQuery::Choices &choices ) const { ASSERT(!"implement DecidedQuery()"); return DecidedQueryResult(); }                                
    virtual TreePtr<Node> GetCoupled() = 0;                                  
    virtual void ResetKey() = 0;     
    virtual void KeyReplace( const TreePtrInterface &x,
                             AgentQuery::Choices choices ) = 0;
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
                             AgentQuery::Choices choices );
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
