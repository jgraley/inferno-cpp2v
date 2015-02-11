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


bool AgentCommon::MatchingDecidedCompare( const TreePtrInterface &x,
                                          bool can_key,
                                          Conjecture &conj ) 
{
    bool r;

    if( can_key )
        coupling_keys->Clear();

    // Only key if the keys are already set to KEYING (which is 
    // the initial value). Keys could be RESTRICTING if we're under
    // a SoftNot node, in which case we only want to restrict.
    if( can_key )
    {
        // Do a two-pass matching process: first get the keys...
        TRACE("doing KEYING pass....\n");
        conj.PrepareForDecidedCompare();
        r = DecidedCompare( x, true, conj );
        TRACE("KEYING pass result %d\n", r );
        if( !r )
            return false;                  // Save time by giving up if no match found
    }
    
    // Now restrict the search according to the couplings
    TRACE("doing RESTRICTING pass....\n");
    conj.PrepareForDecidedCompare();
    r = DecidedCompare( x, false, conj );
    TRACE("RESTRICTING pass result %d\n", r );
    if( !r )
        return false;                  // Save time by giving up if no match found

    // Do not revert match keys if we were successful - keep them for replace
    // and any slave search and replace operations we might do.
    return true;
}


bool AgentCommon::Compare( const TreePtrInterface &x,
                           bool can_key ) 
{
    INDENT("C");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*this);
    TRACE(" can_key=%d \n", (int)can_key);
    //TRACE(**pcontext)(" @%p\n", pcontext);
    
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj;
    bool r;
    while(1)
    {
        // Try out the current conjecture. This will call HandlDecision() once for each decision;
        // HandleDecision() will return the current choice for that decision, if absent it will
        // add the decision and choose the first choice, if the decision reaches the end it
        // will remove the decision.
        r = MatchingDecidedCompare( x, can_key, conj );

        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
            break; // Success
            
        if( !conj.Increment() )
            break; // Failure
    }
    return r;
}


void AgentCommon::KeyReplace()
{    
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


