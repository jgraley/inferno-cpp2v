#ifndef SLAVE_AGENT_HPP
#define SLAVE_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "search_replace.hpp" 

namespace SR
{ 
class SearchReplace;

/// Common implementaiton stuff for slaves
class SlaveAgent : public virtual AgentCommon, 
                   public Engine   
{
public:
    SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search );
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> GetThrough() const = 0;    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;
    virtual void Configure( const Set<Agent *> &agents_already_configured );
    virtual void AgentConfigure( const Engine *e );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;    
};


/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<class PRE_RESTRICTION>
class Slave : public SlaveAgent, 
              public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS

    // SlaveSearchReplace must be constructed using constructor
    Slave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
        through( t ),
        SlaveAgent( sp, rp, is_search )
    {
    }

    TreePtr<PRE_RESTRICTION> through;
    virtual TreePtr<Node> GetThrough() const
    {
        return TreePtr<Node>( through );
    }
};


/// Agent that performs a seperate compare and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class SlaveCompareReplace : public Slave<PRE_RESTRICTION>
{
public:
    SlaveCompareReplace() : Slave<PRE_RESTRICTION>( NULL, NULL, NULL, false ) {}      
    SlaveCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<PRE_RESTRICTION>( t, sp, rp, false ) {}
};


/// Agent that performs a seperate search and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class SlaveSearchReplace : public Slave<PRE_RESTRICTION>
{
public:
    SlaveSearchReplace() : Slave<PRE_RESTRICTION>( NULL, NULL, NULL, true ) {}      
    SlaveSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<PRE_RESTRICTION>( t, sp, rp, true ) {}
};

};

#endif