#ifndef SOFT_AGENT_HPP
#define SOFT_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{ 

/// Common implemtation stuff for soft nodes
class SoftAgent : public virtual AgentCommon
{
public:
    SoftAgent() :
        current_can_key( false )
    {}
    void AgentConfigure( const Engine *e );
    virtual bool DecidedQueryImpl( const TreePtrInterface &x,
                                   bool can_key );
    // This is the main entry-point during replace, but it (confusingly) doesn't
    // call into the soft implementation. Instead, it expects the impl to have
    // already run and to be in a key
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    // Soft nodes should override this to implement their comparison function
    virtual bool MyCompare( const TreePtrInterface &x );
    virtual TreePtr<Node> MyBuildReplace();
    virtual void MyConfigure() {} // No overrides at present
protected: // Call only from the soft node implementation in MyCompare()
    // Compare for child nodes in a normal context (i.e. in which the pattern must match
    // for an overall match to be possible, and so can be used to key a coupling)
    bool NormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern );
    // Compare for child nodes in an abnormal context (i.e. in which the pattern need not match
    // for an overall match to be possible, and so cannot be used to key a coupling)
    bool AbnormalCompare( const TreePtrInterface &x, const TreePtrInterface &pattern );
    TreePtr<Node> *GetContext();
    bool IsCanKey();
    TreePtr<Node> GetCoupled( TreePtr<Node> pattern );
    TreePtr<Node> DoBuildReplace( TreePtr<Node> pattern ) ;
protected:
    bool current_can_key;
};

};

#endif