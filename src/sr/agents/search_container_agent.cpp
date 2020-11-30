#include "search_container_agent.hpp"
#include "../search_replace.hpp" 
#include "../conjecture.hpp" 
#include "link.hpp"
#include "the_knowledge.hpp"

using namespace SR;

//---------------------------------- SearchContainerAgent ------------------------------------    

shared_ptr<PatternQuery> SearchContainerAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    
	pq->RegisterDecision( false ); // Exclusive, please.
	pq->RegisterNormalLink( PatternLink(this, &terminus) );
    
    // Allow subclasses to further restrict
    PatternQueryRestrictions( pq );
    
    return pq;
}


void SearchContainerAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                                XLink base_xlink ) const
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");
    query.Reset();
    
    // Check pre-restriction
    CheckLocalMatch(base_xlink.GetChildX().get());
    
    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(base_xlink)("\n");

    // Get an interface to the container we will search
    // TODO what is keeping pwx alive after this function exits? Are the iterators 
    // doing it? (they are stores in Conjecture). Maybe pwx is just a stateless
    // facade for the iterators and can be abandoned safely?
    shared_ptr<ContainerInterface> pwx = GetContainerInterface( base_xlink );
    
    if( pwx->empty() )
    {
        throw Mismatch();     // The search container is empty, thus terminus could never be matched
    }

    // Get choice from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = query.RegisterDecision( pwx->begin(), pwx->end(), false );
    query.RegisterNormalLink( PatternLink(this, &terminus), GetXLinkFromIterator(base_xlink, thistime) ); // Link into X

    // Let subclasses implement further restrictions
    DecidedQueryRestrictions( query, thistime, base_xlink );
}


bool SearchContainerAgent::ImplHasDNLQ() const
{
    return true;
}


void SearchContainerAgent::KeyReplace( const CouplingKeysMap *coupling_keys )
{
    terminus_key = coupling_keys->at(AsAgent(terminus)).GetChildX();
    SetKey( coupling_keys->at(this) );  
}


TreePtr<Node> SearchContainerAgent::BuildReplaceImpl( CouplingKey keylink ) 
{
    INDENT("#");
    TRACE( "Stuff node: Duplicating at terminus first: keynode=")(*(terminus))
                                                        (", term=")(*(terminus_key))("\n");
    ASSERT(terminus_key);// this could mean replace is being attempted on a SearchContainerAgent in an abnormal context
    TreePtr<Node> term = AsAgent(terminus)->BuildReplace();
    TRACE( "Stuff node: Substituting stuff");
    return DuplicateSubtree(keylink.GetChildX(), terminus_key, term);   
}


//---------------------------------- AnyNode ------------------------------------    

shared_ptr<ContainerInterface> AnyNodeAgent::GetContainerInterface( XLink base_xlink ) const
{ 
    // Note: does not do the flatten every time - instead, the FlattenNode object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new FlattenNode( base_xlink.GetChildX() ) );
}


XLink AnyNodeAgent::GetXLinkFromIterator( XLink base_xlink, ContainerInterface::iterator it ) const
{
    return XLink(base_xlink.GetChildX(), &*it);
}


void AnyNodeAgent::RunDecidedNormalLinkedQueryImpl( DecidedQueryAgentInterface &query,
                                                    XLink base_xlink,
                                                    const SolutionMap *required_links,
                                                    const TheKnowledge *knowledge ) const
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");
    query.Reset();

    // Check pre-restriction
    CheckLocalMatch(base_xlink.GetChildX().get());
    
    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(base_xlink)("\n");
    
    auto terminus_plink = PatternLink(this, &terminus);
    XLink req_terminus_xlink = required_links->at(terminus_plink); 
    query.RegisterNormalLink( terminus_plink, req_terminus_xlink ); // Note: just extracted directly from required_links
    
    const TheKnowledge::Nugget &nugget( knowledge->GetNugget(req_terminus_xlink) );
    if( !nugget.parent_xlink )
        throw NoParentMismatch();                    
    if( nugget.parent_xlink != base_xlink )      
        throw TerminusMismatch();            
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

shared_ptr<ContainerInterface> StuffAgent::GetContainerInterface( XLink base_xlink ) const
{    
    // Note: does not do the walk every time - instead, the Walk object's range is presented
    // to the Conjecture object, which increments it only when trying alternative choice
    return shared_ptr<ContainerInterface>( new Walk( base_xlink.GetChildX(), nullptr, nullptr ) );
}


XLink StuffAgent::GetXLinkFromIterator( XLink base_xlink, ContainerInterface::iterator it ) const
{
    const Walk::iterator *pwtt = dynamic_cast<const Walk::iterator *>(it.GetUnderlyingIterator());
    ASSERT( pwtt );
    return XLink::FromWalkIterator( *pwtt, base_xlink );
}


void StuffAgent::PatternQueryRestrictions( shared_ptr<PatternQuery> pq ) const
{
    if( recurse_restriction )
        pq->RegisterMultiplicityLink( PatternLink(this, &recurse_restriction) );
}


void StuffAgent::DecidedQueryRestrictions( DecidedQueryAgentInterface &query, ContainerInterface::iterator thistime, XLink base_xlink ) const
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
        for( pair<TreePtr<Node>, const TreePtrInterface *> p : pwtt->GetCurrentPath() )
        {
            xpr_ss->elts.push_back( p.second ? XLink(p.first, p.second) : base_xlink );
            TRACE("DQR ")(p.first)(" ")(p.second)("\n");
        }

        XLink xpr_ss_link = XLink::CreateDistinct( xpr_ss ); // Only used in after-pass
        query.RegisterMultiplicityLink( PatternLink(this, &recurse_restriction), xpr_ss_link ); // Links into X     
    }   
}


void StuffAgent::RunDecidedNormalLinkedQueryImpl( DecidedQueryAgentInterface &query,
                                                  XLink base_xlink,
                                                  const SolutionMap *required_links,
                                                  const TheKnowledge *knowledge ) const
{
    INDENT("#");
    ASSERT( this );
    ASSERT( terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");
    query.Reset();

    // Check pre-restriction
    CheckLocalMatch(base_xlink.GetChildX().get());
    
    TRACE("SearchContainer agent ")(*this)(" terminus pattern is ")(*(terminus))(" at ")(base_xlink)("\n");
    
    auto terminus_plink = PatternLink(this, &terminus);
    XLink req_terminus_xlink = required_links->at(terminus_plink); 
    query.RegisterNormalLink( terminus_plink, req_terminus_xlink ); // Note: just extracted directly from required_links
    
    XLink x = req_terminus_xlink;
    bool found = false;
    TreePtr<SubSequence> xpr_ss( new SubSequence() );
    TRACE("Seeking ")(base_xlink)(" in ancestors of ")(req_terminus_xlink)("\n");
    while(true)
    {
        if( x == base_xlink )
        {
            found = true;
            TRACEC("Found ")(x)("\n");
            break;            
        }        
        
        const TheKnowledge::Nugget &nugget( knowledge->GetNugget(x) );
        if( !nugget.parent_xlink )
            break;            
        x = nugget.parent_xlink;
        
        // Putting this here excludes the terminus, as required
        TRACEC("Move to parent ")(x)("\n");
        xpr_ss->elts.push_front( x );      
    }
    if( !found )
    {
        TRACEC("Not found\n");        
        throw TerminusMismatch();
    }
    
    if( recurse_restriction )
    {
        XLink xpr_ss_link = XLink::CreateDistinct( xpr_ss ); // Only used in after-pass
        query.RegisterMultiplicityLink( PatternLink(this, &recurse_restriction), xpr_ss_link ); // Links into X    
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

