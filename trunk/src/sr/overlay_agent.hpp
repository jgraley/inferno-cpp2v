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


template<class PRE_RESTRICTION>
struct Overlay : OverlayAgent, Special<PRE_RESTRICTION>
{
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


template<class PRE_RESTRICTION>
struct Insert : InsertAgent, Special<PRE_RESTRICTION>
{
    SPECIAL_NODE_FUNCTIONS
    Sequence<PRE_RESTRICTION> insert;
    virtual SequenceInterface *GetInsert()  
    {
        return &insert;
    }
};


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


template<class PRE_RESTRICTION>
struct Erase : EraseAgent, Special<PRE_RESTRICTION>
{
    SPECIAL_NODE_FUNCTIONS
    Sequence<PRE_RESTRICTION> erase;
    virtual SequenceInterface *GetErase()  
    {
        return &erase;
    }
};
    
};

#endif