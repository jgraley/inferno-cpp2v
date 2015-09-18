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
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
    virtual void TrackingKey( Agent *from );
    virtual TreePtr<Node> GetThrough() const = 0;    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;
    virtual void Configure( const Set<Agent *> &agents_already_configured, const Engine *master );
    void SetMasterCouplingKeys( CouplingMap &keys );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;   
    
private:    
    /** Walks the tree, avoiding the "search"/"compare" and "replace" members of slaves
        but still recurses through the "through" member. Therefore, it visits all the
        nodes under the same engine as the root. Based on UniqueWalk, so each node only
        visited once. */
    class UniqueWalkNoSlavePattern_iterator : public UniqueWalk::iterator
    {
    public:
        UniqueWalkNoSlavePattern_iterator( TreePtr<Node> &root ) : UniqueWalk::iterator(root) {}        
        UniqueWalkNoSlavePattern_iterator() : UniqueWalk::iterator() {}
        virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const
        {
            return shared_ptr<UniqueWalkNoSlavePattern_iterator>( new UniqueWalkNoSlavePattern_iterator(*this) );
        }      
    private:
        virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
        {
            // We need to create a container of elements of the child.
            if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>( Agent::AsAgent(n) ) )
            {
                // it's a slave, so set up a container containing only "through", not "compare" or "replace"
                shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
                seq->push_back( sa->GetThrough() );
                return seq;
            }
            else
            {
                // it's not a slave, so proceed as for UniqueWalk
                return UniqueWalk::iterator::GetChildContainer(n);
            }
        }
    };

public:
    typedef ContainerFromIterator< UniqueWalkNoSlavePattern_iterator, TreePtr<Node> > UniqueWalkNoSlavePattern;    
    CouplingMap *master_coupling_keys;
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
