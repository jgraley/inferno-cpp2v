#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 
#include "conjecture.hpp" 

using namespace SR;

deque<Agent *> SearchContainerAgent::PatternQuery() const
{
	deque<Agent *> child_agents;
    child_agents.push_back( AsAgent(terminus) );
    // Recurse restriction is abnormal context, so not included
	return child_agents;
}


bool SearchContainerAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
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
            
    return true;
}


void SearchContainerAgent::KeyReplace( const TreePtrInterface &x,
                                       deque<ContainerInterface::iterator> choices )
{
    ASSERT( choices.size() == 1 )("Expected a single choice, choices.size()=%d", choices.size());
    ContainerInterface::iterator thistime = choices.front();
    terminus_key = *thistime;
    DoKey(x);
}


TreePtr<Node> SearchContainerAgent::BuildReplaceImpl( TreePtr<Node> keynode ) 
{
    INDENT("#");
    TRACE( "Stuff node: Duplicating at terminus first: keynode=")(*(terminus))
                                                        (", term=")(*(terminus_key))("\n");
    ASSERT(terminus_key);// this could mean replace is being attempted on a SearchContainerAgent in an abnormal context
    TreePtr<Node> term = AsAgent(terminus)->BuildReplace();
    TRACE( "Stuff node: Substituting stuff");
    return DuplicateSubtree(keynode, terminus_key, term);   
}


shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( TreePtr<Node> x ) const
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new Walk( x, NULL, NULL ) );
}


shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( TreePtr<Node> x ) const
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}

