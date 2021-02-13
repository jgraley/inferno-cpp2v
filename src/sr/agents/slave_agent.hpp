#ifndef SLAVE_AGENT_HPP
#define SLAVE_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "../scr_engine.hpp" 
#include "colocated_agent.hpp"

namespace SR
{ 
class SearchReplace;

/// Slave agents provide an instance of a search and replace engine within 
/// a pattern as an agent in a replace context. This engine acts as a "slave"
/// to the surounding pattern which is termed the "master". During the master's
/// replace operation, the sub-pattern at `through` is used to replace at the 
/// current location. After that subtree has been created, the slave engine
/// operates on the resulting subtree, performing search and replace operations
/// via the `search_pattern` and `replace_pattern` pointers until no more 
/// matches are found (the usual reductive style).  
class SlaveAgent : public virtual ColocatedAgent, 
                   public RequiresSubordinateSCREngine 
{
public:
    SlaveAgent( TreePtr<Node> sp, TreePtr<Node> rp, bool is_search );
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;              
    virtual void KeyForOverlay( Agent *from );
    virtual const TreePtrInterface *GetThrough() const = 0;    
    virtual Block GetGraphBlockInfo() const;
    virtual void SetMyEngine( SCREngine *my_scr_engine );
    void SetMasterCouplingKeys( const CouplingKeysMap &keys );
    virtual TreePtr<Node> BuildReplaceImpl();
    virtual list<Agent *> GetVisibleChildren( Path v ) const;
	virtual SCREngine *GetSCREngine() const { return my_scr_engine; }
	virtual bool IsSearch() const { return is_search; }
	virtual TreePtr<Node> GetSearchPattern() const { return search_pattern; }
	virtual TreePtr<Node> GetReplacePattern() const { return replace_pattern; }

    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;   
    
    CouplingKeysMap master_keys;

private:
    bool is_search;
    SCREngine *my_scr_engine;
};


/// Agent that allows some transformation to run at the corresponding place in the output tree 
template<class PRE_RESTRICTION>
class Slave : public SlaveAgent, 
              public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    // SlaveSearchReplace must be constructed using constructor
    Slave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp, bool is_search ) :
        through( t ),
        SlaveAgent( sp, rp, is_search )
    {
    }

    TreePtr<PRE_RESTRICTION> through;
    virtual const TreePtrInterface *GetThrough() const
    {
        return &through;
    }
};


/// Slave that performs a seperate compare and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class SlaveCompareReplace : public Slave<PRE_RESTRICTION>
{
public:
    SlaveCompareReplace() : Slave<PRE_RESTRICTION>( nullptr, nullptr, nullptr, false ) {}      
    SlaveCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<PRE_RESTRICTION>( t, sp, rp, false ) {}
};


/// Slave that performs a seperate search and replace operation at the corresponding place in the output tree
template<class PRE_RESTRICTION>
class SlaveSearchReplace : public Slave<PRE_RESTRICTION>
{
public:
    SlaveSearchReplace() : Slave<PRE_RESTRICTION>( nullptr, nullptr, nullptr, true ) {}      
    SlaveSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<PRE_RESTRICTION>( t, sp, rp, true ) {}
};

};

#endif
