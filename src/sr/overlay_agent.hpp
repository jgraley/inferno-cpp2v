#ifndef OVERLAY_AGENT_HPP
#define OVERLAY_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

/// Agent that is used in a combined search/replace path to seperate search and replace patterns out again    
class OverlayAgent : public virtual AgentCommon
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    static Sequence<Node> WalkContainerPattern( ContainerInterface &pattern,
                                                bool replacing );
private:
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual TreePtr<Node> GetOverlay() const = 0;    
};


/// Agent that is used in a combined search/replace path to seperate search and replace patterns out again    
template<class PRE_RESTRICTION>
class Overlay : public OverlayAgent, 
                public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> through;
    TreePtr<PRE_RESTRICTION> overlay;
    virtual TreePtr<Node> GetThrough() const 
    {
        return (TreePtr<Node>)through;
    }
    virtual TreePtr<Node> GetOverlay() const
    {
        return (TreePtr<Node>)overlay;
    }
};


/// Agent used in a container pattern to add a new element during replace
class InsertAgent : public virtual AgentCommon
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
private:
    friend class OverlayAgent;
    virtual SequenceInterface *GetInsert() = 0;    
};


/// Agent used in a container pattern to add a new element during replace
template<class PRE_RESTRICTION>
class Insert : public InsertAgent, 
               public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    Sequence<PRE_RESTRICTION> insert;
    virtual SequenceInterface *GetInsert()  
    {
        return &insert;
    }
};


/// Agent used in a container pattern to match an element then discard it during replace
class EraseAgent : public virtual AgentCommon
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
private:
    friend class OverlayAgent;
    virtual SequenceInterface *GetErase() = 0;    
};


/// Agent used in a container pattern to match an element then discard it during replace
template<class PRE_RESTRICTION>
class Erase : public EraseAgent,
              public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    Sequence<PRE_RESTRICTION> erase;
    virtual SequenceInterface *GetErase()  
    {
        return &erase;
    }
};
    
};

#endif