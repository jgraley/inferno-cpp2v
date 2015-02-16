#ifndef SOFT_AGENT_HPP
#define SOFT_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

// Tell soft nodes that a compare run is beginning and it can flush any caches it may have
/// Utility for agents that allow a flush to be requested
class Flushable
{
public:    
    virtual void FlushCache() {}
};


/// Common implemtation stuff for soft nodes
class SoftAgent : Flushable,
                  public virtual AgentCommon
{
public:
    SoftAgent() :
        current_can_key( false ),
        current_conj( NULL )
    {}
    void Configure( const CompareReplace *s, CouplingKeys *c );
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    // This entrypoint is actually called by the main engine before the replace
    // and the results are put into a key. This one calls into the impl.
    virtual void KeyReplace(); 
    // This is the main entry-point during replace, but it (confusingly) doesn't
    // call into the soft implementation. Instead, it expects the impl to have
    // already run and to be in a key
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
    // Called when coupled, dest is coupling key TODO route through "my" version
    virtual TreePtr<Node> GetOverlayPattern();
    // Soft nodes should override this to implement their comparison function
    virtual bool MyCompare( const TreePtrInterface &x );
    virtual TreePtr<Node> MyBuildReplace();
    virtual void MyConfigure() {}
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
    CouplingKeys *GetCouplingKeys();
    TreePtr<Node> DoBuildReplace( TreePtr<Node> pattern ) ;
protected:
    bool current_can_key;
    Conjecture *current_conj; 
};

};

#endif