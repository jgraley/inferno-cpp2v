#ifndef OVERLAY_AGENT_HPP
#define OVERLAY_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Agent that is used in a combined search/replace path to seperate search 
/// and replace sub-patterns. During search, the `through` pointer will be 
/// followed and the sub-pattern found there must match. During replace, the 
/// sub-pattern under `overlay` is used to reconstruct the output tree.
/// If the replace pattern under `overlay` contains wildcards, these will 
/// be overlayed over the pattern under `through`, to the extent that this
/// is unambiguous and the structures of the two subtrees match.
class OverlayAgent : public virtual AgentCommon
{
public:
    virtual void PatternQueryImpl() const;
    virtual bool DecidedQueryImpl( const TreePtrInterface &x ) const;
	virtual void GetGraphAppearance( bool *bold, string *text, string *shape ) const;
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
private:
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual TreePtr<Node> GetOverlay() const = 0;    
};


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
