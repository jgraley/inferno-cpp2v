#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include <set>
#include <boost/type_traits.hpp>
 
namespace SR
{ 
class Conjecture;
class SpecialBase;
class CompareReplace;
 
/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj ) = 0;
    virtual bool Compare( const TreePtrInterface &x,
                          bool can_key = false,
                          Conjecture *conj = NULL ) = 0;
    virtual TreePtr<Node> GetCoupled() = 0;                                  
    virtual void ResetKey() = 0;     
    virtual void KeyReplace() = 0;
    virtual void TrackingKey( Agent *from ) = 0;
    virtual TreePtr<Node> BuildReplace() = 0;
	virtual void AgentConfigure( const CompareReplace *s ) = 0;
	static Agent *AsAgent( TreePtr<Node> node );
};

// - Configure
// - Pre-restriction
// - Keying (default case)
// - Compare and MAtchingDecidedCompare rings
/// Implments misc functionality common to all or most agents
class AgentCommon : public Agent
{
protected:
    // Base class for coupling keys; this deals with individual node matches, and also with stars
    // by means of pointing "root" at a SubCollection or SubSequence
    /// General holder for coupling keys, remembers important details of partil or full matches
    class Key
    {
    public:
        virtual ~Key(){}  // be a virtual hierarchy
        TreePtr<Node> root; // Input program node for this coupling
    };        

public:
    AgentCommon();
    void AgentConfigure( const CompareReplace *s );
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    void DoKey( TreePtr<Node> x );
    void DoKey( shared_ptr<Key> key );
    TreePtr<Node> GetCoupled();                                  
    virtual shared_ptr<Key> GetKey();
    void ResetKey();                                  

    virtual void KeyReplace();
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> BuildReplace();
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() ) = 0;
    bool MatchingDecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    bool Compare( const TreePtrInterface &x,
                  bool can_key = false,
                          Conjecture *conj = NULL );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
protected:
    const CompareReplace *sr;
    
private:    
    shared_ptr<Key> coupling_key;    
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