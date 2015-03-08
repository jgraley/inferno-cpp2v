#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 
#include "conjecture.hpp" 

using namespace SR;

bool SearchContainerAgent::DecidedCompareImpl( const TreePtrInterface &x,
                                               bool can_key,
                                               Conjecture &conj )
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    // TODO what is keeping pwx alive after this funciton exits? Are the iterators 
    // doing it? (they are stores in Conjecture). Maybe pwx is just a stateless
    // facade for the iterators and can be abandoned safely?
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( x );
    
    // Get choice from conjecture about where we are in the walk
    ContainerInterface::iterator thistime = conj.HandleDecision( pwx->begin(), pwx->end() );
    if( thistime == (ContainerInterface::iterator)(pwx->end()) )
        return false; // ran out of choices

    // Try out comparison at this position
    TRACE("Trying terminus ")(**thistime);
    bool r = Agent::AsAgent(terminus)->DecidedCompare( *thistime, can_key, conj );
    if( !r )
        return false;
        
    if( TreePtr<Node> keynode = GetCoupled() )
    {
        SimpleCompare sc;
        if( sc( x, keynode ) == false )
            return false;
    }
    
    // If we got this far, do the couplings
    if( can_key )
    {
        shared_ptr<TerminusKey> key( new TerminusKey );
        key->root = x;
        key->terminus = *thistime;
        shared_ptr<Key> sckey( key );
        TRACE("Matched, so keying search container type ")(*this)(" for ")(*x);
        DoKey( sckey );    
    }
    return r;
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


StuffAgent::StuffAgent() : 
    recurse_filter( this )
{
}


StuffAgent::RecurseFilter::RecurseFilter( StuffAgent *a ) :
    agent(a)
{
}


bool StuffAgent::RecurseFilter::IsMatch( TreePtr<Node> context,       
                                         TreePtr<Node> root )
{
    if( agent->recurse_restriction )
        return AsAgent(agent->recurse_restriction)->AbnormalCompare(root);
    else
        return true;
}


shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( TreePtr<Node> x )
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new Walk( x, NULL, &recurse_filter ) );
}


shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( TreePtr<Node> x )
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}

