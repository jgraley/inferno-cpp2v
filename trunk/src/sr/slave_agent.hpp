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

class SlaveAgent : public virtual InPlaceTransformation, 
                   public virtual AgentCommon 
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual void ConfigureImpl( const Set<Agent *> &agents_already_configured ) = 0; // For master to trigger configuration
    virtual void Configure( const CompareReplace *s, CouplingKeys *c ) = 0;
};


template<typename ALGO>
class SlaveIntermediate : public SlaveAgent, 
                          public ALGO                                 
{
public:
    SlaveIntermediate( TreePtr<Node> sp, TreePtr<Node> rp ) :
        ALGO( sp, rp, false )
    {}
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const
    {
        labels->push_back("through");
        links->push_back(GetThrough());
        ALGO::GetGraphInfo( labels, links );
    }
    virtual void ConfigureImpl( const Set<Agent *> &agents_already_configured )
    {
        ALGO::ConfigureImpl(agents_already_configured);
    }       
    virtual void Configure( const CompareReplace *s, CouplingKeys *c )
    {
        AgentCommon::Configure( s, c );
        ALGO::coupling_keys.SetMaster( c );  
        ALGO::master_ptr = s;
    }       
};


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


// Partial specialisation is an arse in C++
template<class PRE_RESTRICTION>
class SlaveCompareReplace : public Slave<CompareReplace, PRE_RESTRICTION>
{
public:
    SlaveCompareReplace() : Slave<CompareReplace, PRE_RESTRICTION>( NULL, NULL, NULL ) {}      
    SlaveCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<CompareReplace, PRE_RESTRICTION>( t, sp, rp ) {}
};


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