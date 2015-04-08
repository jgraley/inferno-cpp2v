#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 
#include "conjecture.hpp" 

using namespace SR;

bool SearchContainerAgent::DecidedQueryImpl( const TreePtrInterface &x,
                                             bool can_key )
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    // Check pre-restriction
    if( !IsLocalMatch(x.get()) )        
        return false;

    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    // TODO what is keeping pwx alive after this funciton exits? Are the iterators 
    // doing it? (they are stores in Conjecture). Maybe pwx is just a stateless
    // facade for the iterators and can be abandoned safely?
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( x );
    
    if( pwx->empty() )
    {
        return false; // The search container is empty, thus terminus could never be matched
    }
    
    // Get choice from conjecture about where we are in the walk
    ContainerInterface::iterator thistime = HandleDecision( pwx->begin(), pwx->end() );

    // Where a recurse restriction is in use, apply it to all the recursion points
    // underlying the current iterator, thistime.
    if( recurse_restriction )
    {
        // See if we are looking at a walk iterator
        const Walk::iterator *pwtt = dynamic_cast<const Walk::iterator *>(thistime.GetUnderlyingIterator());
        ASSERT(pwtt)("recurse_restriction set on non-Stuff node (probably AnyNode)");    

        // Check all the nodes that we recursed through in order to get here
        FOREACH( TreePtr<Node> n, pwtt->GetPath() )
            RememberLocalLink( true, AsAgent(recurse_restriction), n );
    }
    
    // Try out comparison at this position
    TRACE("Trying terminus ")(**thistime)("\n");
    RememberLink( false, AsAgent(terminus), *thistime );
            
    // If we got this far, do the couplings
    if( can_key )
    {
        shared_ptr<TerminusKey> key( new TerminusKey );
        key->root = x;
        key->terminus = *thistime;
        shared_ptr<Key> sckey( key );
        TRACE("Matched, so keying search container type ")(*this)(" for ")(*x)("\n");
        DoKey( sckey );    
    }
    return true;
}


void SearchContainerAgent::KeyReplace( const TreePtrInterface &x,
                                       deque<ContainerInterface::iterator> choices )
{
    if(x)
    {
        ASSERT( choices.size() == 1 );
        ContainerInterface::iterator thistime = choices.front();
        shared_ptr<Key> key = GetKey();
        shared_ptr<TerminusKey> stuff_key = dynamic_pointer_cast<TerminusKey>(key);
        ASSERT( stuff_key->root == x ); 
        ASSERT( stuff_key->terminus == *thistime );
    }
}


TreePtr<Node> SearchContainerAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("#");
    // SearchContainer.
    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
    // so come out of substitution. Done as tail recursion so that we already duplicated
    // the terminus key, and can just overlay the terminus replace pattern.
    shared_ptr<Key> key = GetKey();
    ASSERT( key->root==keynode );    // Check we got the same keynode passed in as we found in the couplins structure
    shared_ptr<TerminusKey> stuff_key = dynamic_pointer_cast<TerminusKey>(key);
    ASSERT( stuff_key );

    TRACE( "Stuff node: Duplicating at terminus first: keynode=")(*(terminus))
                                                        (", term=")(*(stuff_key->terminus))("\n");
    TreePtr<Node> term = AsAgent(terminus)->BuildReplace();
    TRACE( "Stuff node: Substituting stuff");
    return DuplicateSubtree(stuff_key->root, stuff_key->terminus, term);   
}


shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( TreePtr<Node> x )
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new Walk( x, NULL, NULL ) );
}


shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( TreePtr<Node> x )
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}

