#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "../conjecture.hpp"
#include "../boolean_evaluator.hpp"
#include "../query.hpp"
#include "common/mismatch.hpp"
#include "../equivalence.hpp"

#include <vector>
#include <boost/type_traits.hpp>
#include <functional>

namespace SR
{ 
class SpecialBase;
class SCREngine;
class Agent;
class TheKnowledge;

/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    class NormalLinksMismatch : public Mismatch
    {
    };
    
    class CouplingMismatch : public Mismatch
    {
    };
    
    Agent& operator=(Agent& other);
	virtual void AgentConfigure( const SCREngine *master_scr_engine ) = 0;

    /// List the Agents reached via links during search
    virtual shared_ptr<PatternQuery> GetPatternQuery() const = 0;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const = 0;
    /// Produce info about an Agent given location (x) and a vector of choices (conj). 
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  XLink base_xlink ) const = 0;                                                
    typedef function<shared_ptr<DecidedQuery>()> QueryLambda;
    virtual QueryLambda StartNormalLinkedQuery( XLink base_xlink,
                                                const SolutionMap *required_links,
                                                const TheKnowledge *knowledge,
                                                bool force_common = false ) const = 0;
    virtual QueryLambda TestStartNormalLinkedQuery( XLink base_xlink,
                                                    const SolutionMap *required_links,
                                                    const TheKnowledge *knowledge ) const = 0;
    virtual void CouplingQuery( multiset<XLink> candidate_links ) = 0;                                       
    virtual unordered_set<XLink> ExpandNormalDomain( const unordered_set<XLink> &xlinks ) = 0;
    // function<XLink(XLink)> deduplicator

    virtual CouplingKey GetKey() = 0;                                  
    virtual void ResetKey() = 0;     
    virtual void KeyReplace( const CouplingKeysMap *coupling_keys ) = 0;
    virtual void TrackingKey( Agent *from ) = 0;
    virtual TreePtr<Node> BuildReplace() = 0;
    virtual TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                            TreePtr<Node> source_terminus = TreePtr<Node>(),
                                            TreePtr<Node> dest_terminus = TreePtr<Node>() ) const = 0;
    virtual shared_ptr<ContainerInterface> GetVisibleChildren() const = 0;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const = 0;
		
	static Agent *AsAgent( shared_ptr<Node> node );
	static const Agent *AsAgentConst( shared_ptr<const Node> node );
    // Note the constness: thou shalt not try to mutate the agent via this function
    virtual shared_ptr<const Node> GetPatternPtr() const = 0;
};


// - Configure
// - Pre-restriction
// - Keying (default case)
// - Compare and MatchingDecidedCompare rings
/// Implments misc functionality common to all or most agents
class AgentCommon : public Agent
{
public:
    AgentCommon();
    virtual void AgentConfigure( const SCREngine *master_scr_engine );
    virtual shared_ptr<ContainerInterface> GetVisibleChildren() const;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const;
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  XLink base_xlink ) const;                                                
    virtual void DecidedNormalLinkedQuery( DecidedQuery &query,
                                           XLink base_xlink,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                              
    void CheckMatchingLinks( const DecidedQueryCommon::Links &mut_links, 
                             const DecidedQueryCommon::Links &ref_links ) const;
    virtual QueryLambda StartNormalLinkedQuery( XLink base_xlink,
                                                const SolutionMap *required_links,
                                                const TheKnowledge *knowledge,
                                                bool force_common = false ) const;
    virtual QueryLambda TestStartNormalLinkedQuery( XLink base_xlink,
                                                    const SolutionMap *required_links,
                                                    const TheKnowledge *knowledge ) const;
    virtual void CouplingQuery( multiset<XLink> candidate_links );                                       
    virtual unordered_set<XLink> ExpandNormalDomain( const unordered_set<XLink> &xlinks ) { return unordered_set<XLink>(); /* implement in agents that can expand the domain */ }

protected:
    void SetKey( CouplingKey x );

public:
    CouplingKey GetKey();                                  
    void ResetKey();    
    virtual void KeyReplace( const CouplingKeysMap *coupling_keys );
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplace();
    virtual TreePtr<Node> BuildReplaceImpl( CouplingKey keylink=CouplingKey() );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
protected:                                  
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const = 0;                                      
    const SCREngine *master_scr_engine;    
    shared_ptr<PatternQuery> pattern_query;
			
private:    
    CouplingKey coupling_key;    
    int num_decisions;
    EquivalenceRelation equivalence_relation;
};


class AgentCommonNeedSCREngine : public AgentCommon
{
public:
	virtual bool IsSearch() const = 0;
	virtual TreePtr<Node> GetSearchPattern() const = 0;
	virtual TreePtr<Node> GetReplacePattern() const = 0;
    virtual void AgentConfigure( const SCREngine *master_scr_engine, SCREngine *my_engine ) = 0;
    virtual void SetMasterCouplingKeys( const CouplingKeysMap &keys ) = 0;
};


class AgentCommonDomainExtender : public AgentCommon
{
    virtual unordered_set<XLink> ExpandNormalDomain( const unordered_set<XLink> &xlinks );
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

};
#endif
