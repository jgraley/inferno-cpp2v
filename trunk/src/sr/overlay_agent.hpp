#ifndef OVERLAY_AGENT_HPP
#define OVERLAY_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Agent that is used in a combined search/replace path to seperate search and replace patterns out again    
class OverlayAgent : public virtual AgentCommon
{
public:
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
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

};

#endif
