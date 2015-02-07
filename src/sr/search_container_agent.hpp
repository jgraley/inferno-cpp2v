#ifndef SEARCH_CONTAINER_AGENT_HPP
#define SEARCH_CONTAINER_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

struct TerminusKey : Key // TODO put in TerminusBase
{
    TreePtr<Node> terminus;
};


struct TerminusBase : virtual Node 
{
    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
};
    

class SearchContainerAgent : public TerminusBase,
                             public virtual AgentCommon
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x ) = 0;
};


// The Stuff wildcard can match a truncated subtree with special powers as listed by the members
struct StuffAgent : virtual Node, // TODO no need
                    public SearchContainerAgent
{
    StuffAgent();
    TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
    CompareReplace * const recurse_comparer;
    bool one_level; // TODO lose this "one_level" thing: AnyNode now does that
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x );
};


template<class PRE_RESTRICTION>
struct Stuff : StuffAgent, Special<PRE_RESTRICTION> 
{
    // Do the itemiser by hand since it gets confused by the CompareReplace object   
    virtual vector< Itemiser::Element * > Itemise( const Itemiser *itemise_object = 0 ) const
    {
        vector< Itemiser::Element * > v;
        v.push_back( (Itemiser::Element *)(&recurse_restriction) );
        v.push_back( (Itemiser::Element *)(&terminus) );
        return v;
    }
    virtual Itemiser::Element *ItemiseIndex( int i ) const  
    { 
        return Itemise()[i];
    } 
    virtual int ItemiseSize() const  
    { 
        return Itemise().size();
    }
};


struct AnyNodeAgent : virtual Node, // TODO no need?
                      public SearchContainerAgent
{
    virtual shared_ptr<ContainerInterface> GetContainerInterface( TreePtr<Node> x );
};


template<class PRE_RESTRICTION>
struct AnyNode : AnyNodeAgent, Special<PRE_RESTRICTION> 
{
    SPECIAL_NODE_FUNCTIONS
};

};

#endif