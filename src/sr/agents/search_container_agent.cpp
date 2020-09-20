#include "search_container_agent.hpp"
#include "helpers/simple_compare.hpp"
#include "../search_replace.hpp" 
#include "../conjecture.hpp" 
#include "link.hpp"

using namespace SR;

//---------------------------------- SearchContainerAgent ------------------------------------    

shared_ptr<PatternQuery> SearchContainerAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    
	pq->RegisterDecision( false ); // Exclusive, please.
	pq->RegisterNormalLink( &terminus );
    
    // Allow subclasses to further restrict
    PatternQueryRestrictions( pq );
    
    return pq;
}


void SearchContainerAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                                TreePtr<Node> x ) const
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(x.get());
    
    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    // TODO what is keeping pwx alive after this function exits? Are the iterators 
    // doing it? (they are stores in Conjecture). Maybe pwx is just a stateless
    // facade for the iterators and can be abandoned safely?
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( x );
    
    if( pwx->empty() )
    {
        throw Mismatch();     // The search container is empty, thus terminus could never be matched
    }

    // Get choice from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = query.RegisterDecision( pwx->begin(), pwx->end(), false );
    query.RegisterNormalLink( &terminus, *thistime ); // Link into X

    // Let subclasses implement further restrictions
    DecidedQueryRestrictions( query, thistime );
}


void SearchContainerAgent::KeyReplace( const CouplingMap *coupling_keys )
{
    terminus_key = coupling_keys->at(AsAgent(terminus));
    SetKey( coupling_keys->at(this) );  
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


void StuffAgent::PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const
{
    if( recurse_restriction )
        pq->RegisterMultiplicityLink( &recurse_restriction );
}


void StuffAgent::DecidedQueryRestrictions( DecidedQueryAgentInterface &query, ContainerInterface::iterator thistime ) const
{
    // Where a recurse restriction is in use, apply it to all the recursion points
    // underlying the current iterator, thistime.
    if( recurse_restriction )
    {
        TreePtr<SubSequence> xpr_ss( new SubSequence() );

        // See if we are looking at a walk iterator
        const Walk::iterator *pwtt = dynamic_cast<const Walk::iterator *>(thistime.GetUnderlyingIterator());
        ASSERT(pwtt)("Failed to get Walk::iterator out of the decision iterator");    

        // Check all the nodes that we recursed through in order to get here
        FOREACH( TreePtr<Node> n, pwtt->GetCurrentPath() )
            xpr_ss->push_back( n );

        query.RegisterMultiplicityLink( &recurse_restriction, xpr_ss ); // Links into X     
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

