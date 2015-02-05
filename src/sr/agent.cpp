#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

using namespace SR;

void AgentCommon::Configure( const CompareReplace *s, CouplingKeys *c )
{
    ASSERT(s);
    ASSERT(c);
    sr = s;
    coupling_keys = c;
    // TODO recursively configure children
}


bool AgentCommon::DecidedCompare( const TreePtrInterface &x,
                                  bool can_key,
                                  Conjecture &conj )
{
    INDENT;
    ASSERT(sr)("Agent ")(*this)(" at appears not to have been configured, since sr is NULL");
    ASSERT(coupling_keys);
    ASSERT( x ); // Target must not be NULL
    ASSERT(this);
    
    // Check whether the present node matches. Do this for all nodes: this will be the local
    // restriction for normal nodes and the pre-restriction for special nodes (based on
    // how IsLocalMatch() has been overridden.
    if( !IsLocalMatch(x.get()) )
    {
        return false;
    }
    
    bool match = DecidedCompareImpl( x, can_key, conj );
    if( !match )
        return false;
    
    if( TreePtr<Node> keynode = coupling_keys->GetCoupled( this ) )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
    
    // Note that if the DecidedCompareImpl() already keyed, then this does nothing.
    if( can_key )
        coupling_keys->DoKey( x, this );  

    return true;
}


TreePtr<Node> AgentCommon::BuildReplace( TreePtr<Node> keynode )
{
    INDENT;
    ASSERT(this);
    ASSERT(sr)("Agent ")(*this)(" at appears not to have been configured, since sr is NULL");
    ASSERT(coupling_keys);
    
    // See if the pattern node is coupled to anything. The keynode that was passed
    // in is just a suggestion and will be overriden if we are keyed.
    shared_ptr<Key> key = coupling_keys->GetKey( this );
    if( key )
        keynode = key->root;

    return BuildReplaceImpl( keynode );    
}


