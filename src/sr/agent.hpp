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
class StuffBase;
class StarBase;
class SlaveBase;
class SearchContainerBase;
class SearchReplace;
 
class Agent : public virtual Traceable,
              public virtual Node
{
public:
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj ) = 0;
    virtual bool Compare( const TreePtrInterface &x,
                          bool can_key = false ) = 0;
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> keynode=TreePtr<Node>() ) = 0;
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
class AgentCommon : public Agent
{
public:
    AgentCommon() :sr(NULL), coupling_keys(NULL) {}
    void Configure( const CompareReplace *s, CouplingKeys *c );
    virtual bool DecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    virtual TreePtr<Node> BuildReplace( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj ) = 0;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() ) = 0;
    bool MatchingDecidedCompare( const TreePtrInterface &x,
                                 bool can_key,
                                 Conjecture &conj );
    bool Compare( const TreePtrInterface &x,
                  bool can_key = false );
protected:
    const CompareReplace *sr;
    CouplingKeys *coupling_keys;
};


};
#endif