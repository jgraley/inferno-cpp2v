#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "search_replace.hpp" 
#include "conjecture.hpp" 

using namespace SR;

//---------------------------------- SearchContainerAgent ------------------------------------    

PatternQueryResult SearchContainerAgent::PatternQuery() const
{
    PatternQueryResult r;
	r.AddLink( false, AsAgent(terminus) );
    return r;
}


void SearchContainerAgent::DecidedQuery( QueryAgentInterface &query,
                                         const TreePtrInterface *px ) const
{
    INDENT("#");
    ASSERT( this );
    ASSERT(px);
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");
    query.Reset();
    
    // Check pre-restriction
    if( !IsLocalMatch(px->get()) )        
    {
        query.AddLocalMismatch();  
        return;
    }

    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(**px)("\n");

    // Get an interface to the container we will search
    // TODO what is keeping pwx alive after this function exits? Are the iterators 
    // doing it? (they are stores in Conjecture). Maybe pwx is just a stateless
    // facade for the iterators and can be abandoned safely?
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( *px );
    
    if( pwx->empty() )
    {
        query.AddLocalMismatch();   // The search container is empty, thus terminus could never be matched
        return;
    }

    // Get choice from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = query.AddDecision( pwx->begin(), pwx->end(), false );
    query.AddLink( false, AsAgent(terminus), &*thistime );

    // Let subclasses implement further restrictions
    DecidedQueryRestrictions( query, thistime );
}


void SearchContainerAgent::KeyReplace( const TreePtrInterface &x,
                                       QueryCommonInterface::Choices choices )
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


//---------------------------------- AnyNode ------------------------------------    

shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( TreePtr<Node> x ) const
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}


void AnyNodeAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The AnyNode node appears as a small circle with a ? sign in it. The terminus block emerges from the
	// right of the circle. ? implies the tendancy to match exactly one thing.
	*bold = true;
	*shape = "circle";
	*text = string("?"); 
}

//---------------------------------- Stuff ------------------------------------    

shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( TreePtr<Node> x ) const
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new Walk( x, NULL, NULL ) );
}


void StuffAgent::PatternQueryRestrictions( PatternQueryResult &r ) const
{
    if( recurse_restriction )
        r.AddLink( true, AsAgent(recurse_restriction) );
}


void StuffAgent::DecidedQueryRestrictions( QueryAgentInterface &query, ContainerInterface::iterator thistime ) const
{
    // Where a recurse restriction is in use, apply it to all the recursion points
    // underlying the current iterator, thistime.
    if( recurse_restriction )
    {
        // See if we are looking at a walk iterator
        const Walk::iterator *pwtt = dynamic_cast<const Walk::iterator *>(thistime.GetUnderlyingIterator());
        ASSERT(pwtt)("Failed to get Walk::iterator out of the decision iterator");    

        // Check all the nodes that we recursed through in order to get here
        FOREACH( TreePtr<Node> n, pwtt->GetPath() )
            query.AddLocalLink( true, AsAgent(recurse_restriction), n );
    }   
}


void StuffAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The Stuff node appears as a small circle with a # character inside it. The terminus block emerges from the
	// right of the circle. If there is a recurse restriction the circle is egg-shaped and the restriction block 
	// emerges from the top of the egg shape. # is chosen (as is the name Stuff) for its similarity to * because
	// the nodes are both able to wildcard multiple nodes in the tree.
	*bold = true;
	if( recurse_restriction )
		*shape = "egg";
	else
		*shape = "circle";
	*text = string("#"); 
}

