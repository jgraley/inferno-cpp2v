#ifndef SLAVE_AGENT_HPP
#define SLAVE_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

/// Common implementaiton stuff for slaves
class SlaveAgent : public virtual InPlaceTransformation, 
                   public virtual AgentCommon 
{
public:
    SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp );
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual void SetReplaceKey( shared_ptr<Key> key );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual void Configure( const Set<Agent *> &agents_already_configured ) = 0; // For master to trigger configuration
    virtual void AgentConfigure( const CompareReplace *s, CouplingKeys *c ) = 0;
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;    
};


/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<typename ALGO>
class SlaveIntermediate : public SlaveAgent, 
                          public ALGO                                 
{
public:
    SlaveIntermediate( TreePtr<Node> sp, TreePtr<Node> rp ) :
        SlaveAgent( sp, rp )
    {        
    }
    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const
    {
        labels->push_back("through");
        links->push_back(GetThrough());
        ALGO::GetGraphInfo( labels, links );
    }
    virtual void Configure( const Set<Agent *> &agents_already_configured )
    {
        ALGO::Configure(search_pattern, replace_pattern, agents_already_configured);
    }       
    virtual void AgentConfigure( const CompareReplace *s, CouplingKeys *c )
    {
        AgentCommon::AgentConfigure( s, c );
        ALGO::coupling_keys.SetMaster( c );  
        ALGO::master_ptr = s;
    }       
};


/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<typename ALGO, class PRE_RESTRICTION>
class Slave : public SlaveIntermediate<ALGO>, 
              public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS

    // SlaveSearchReplace must be constructed using constructor
    Slave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp ) :
        through( t ),
        SlaveIntermediate<ALGO>( sp, rp )
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
class SlaveCompareReplace : public Slave<CompareReplace, PRE_RESTRICTION>
{
public:
    SlaveCompareReplace() : Slave<CompareReplace, PRE_RESTRICTION>( NULL, NULL, NULL ) {}      
    SlaveCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<CompareReplace, PRE_RESTRICTION>( t, sp, rp ) {}
};


/// Agent that performs a seperate search and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class SlaveSearchReplace : public Slave<SearchReplace, PRE_RESTRICTION>
{
public:
    SlaveSearchReplace() : Slave<SearchReplace, PRE_RESTRICTION>( NULL, NULL, NULL ) {}      
    SlaveSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<SearchReplace, PRE_RESTRICTION>( t, sp, rp ) {}
};

};

#endif