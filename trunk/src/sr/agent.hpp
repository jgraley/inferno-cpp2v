#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"
#include <set>
#include <boost/type_traits.hpp>
 
namespace SR
{ 
class Conjecture;
class SpecialBase;
class SearchReplace;
 
/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj ) = 0;
    virtual bool Compare( const TreePtrInterface &x,
                          bool can_key = false ) = 0;
    virtual void KeyReplace() = 0;
    virtual void SetReplaceKey( TreePtr<Node> keynode ) = 0;
    virtual TreePtr<Node> BuildReplace() = 0;
	virtual void Configure( const CompareReplace *s, CouplingKeys *c ) = 0;
	static Agent *AsAgent( TreePtr<Node> node )
	{
		ASSERT( node )("Called AsAgent(")(node)(") with NULL TreePtr");
		Agent *agent = dynamic_cast<Agent *>(node.get());
		ASSERT( agent )("Called AsAgent(")(*node)(") with non-Agent");
		return agent;
	}
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
    void Configure( const CompareReplace *s, CouplingKeys *c );
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    virtual void SetReplaceKey( TreePtr<Node> keynode );
    virtual void KeyReplace();
    virtual TreePtr<Node> BuildReplace();
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj ) = 0;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() ) = 0;
    bool MatchingDecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    bool Compare( const TreePtrInterface &x,
                  bool can_key = false );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
protected:
    const CompareReplace *sr;
    CouplingKeys *coupling_keys;
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


};
#endif
