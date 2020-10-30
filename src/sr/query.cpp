#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/agent.hpp"
#include "link.hpp"

#include <inttypes.h>

using namespace SR;

//#define KEEP_WHODAT_INFO

// For debugging
#ifdef KEEP_WHODAT_INFO
#define WHODAT() __builtin_extract_return_addr (__builtin_return_address (0))
#else
#define WHODAT() nullptr
#endif
 
PatternQuery::PatternQuery( const Agent *base_agent_ ) :
    base_agent( base_agent_ )
{
}
 
 
void PatternQuery::clear()
{
    normal_links.clear();
    abnormal_links.clear();
    multiplicity_links.clear();
    evaluator = shared_ptr<BooleanEvaluator>();
}


void PatternQuery::RegisterDecision( bool inclusive )
{
    Decision d;
    d.inclusive = inclusive;
    decisions.push_back(d);
}

 
void PatternQuery::RegisterNormalLink( const TreePtrInterface *ppattern )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    PatternLink b( ba, ppattern, WHODAT() );
    normal_links.push_back( b );        
}


void PatternQuery::RegisterAbnormalLink( const TreePtrInterface *ppattern )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    PatternLink b( ba, ppattern, WHODAT() );
    abnormal_links.push_back( b );       
}


void PatternQuery::RegisterMultiplicityLink( const TreePtrInterface *ppattern )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    PatternLink b( ba, ppattern, WHODAT() );
    multiplicity_links.push_back( b );       
}


PatternQuery::Links PatternQuery::GetAllLinks() const
{
    Links links;
    for( auto link : GetNormalLinks() )
        links.push_back( link );
    for( auto link : GetAbnormalLinks() )
        links.push_back( link );
    for( auto link : GetMultiplicityLinks() )
        links.push_back( link );
    return links;
}


const TreePtr<DecidedQueryCommon::MMAX> DecidedQueryCommon::MMAX_Node = MakeTreePtr<DecidedQueryCommon::MMAX>();


DecidedQuery::DecidedQuery( shared_ptr<const PatternQuery> pq ) :
    base_agent( pq->GetBaseAgent() ),
    decisions( pq->GetDecisions().size() ),
    next_decision( decisions.begin() ), 
    choices( pq->GetDecisions().size() ),
    next_choice( choices.begin() ) 
{
    CompleteDecisionsWithEmpty();
}


void DecidedQuery::Start()
{
    for( Range &r : decisions )
        r = empty_range;
}


void DecidedQuery::RegisterNormalLink( const TreePtrInterface *ppattern, XLink xlink )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    LocatedLink link( ba, ppattern, xlink );
    normal_links.push_back( link );        
}


void DecidedQuery::RegisterAbnormalLink( const TreePtrInterface *ppattern, XLink xlink )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    LocatedLink link( ba, ppattern, xlink );
    abnormal_links.push_back( link );
}


void DecidedQuery::RegisterMultiplicityLink( const TreePtrInterface *ppattern, XLinkMultiplicity xlink )
{
    shared_ptr<const Node> bpp = base_agent->GetPatternPtr();
    const Agent *ba = Agent::AsAgentConst(bpp);
    LocatedLink link( ba, ppattern, xlink );
    multiplicity_links.push_back( link );
}


void DecidedQuery::RegisterAlwaysMatchingLink( const TreePtrInterface *ppattern )
{ 
    LocatedLink link( base_agent, ppattern, MMAX_Node, WHODAT() );
    normal_links.push_back( link );      
}


void PatternQuery::RegisterEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	


void DecidedQuery::RegisterEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	
        
        
void DecidedQuery::Invalidate( int bc )
{
    // TODO may not need all thes preconditions
    ASSERT( !choices.empty() );
    ASSERT( bc >= 0 );
    ASSERT( bc < choices.size() );
    if( next_decision > decisions.begin() + bc ) 
        next_decision = decisions.begin() + bc; 
  
    //decisions.pop_back();    
    choices[bc].mode = Choice::BEGIN;    
}


void DecidedQuery::SetChoice( int bc, Choice newc )
{
    ASSERT( !choices.empty() );
    ASSERT( bc >= 0 );
    ASSERT( bc < choices.size() );
    choices[bc] = newc;
}


void DecidedQuery::PushBackChoice( Choice newc )
{
    choices.push_back(newc);
}


void DecidedQuery::EnsureChoicesHaveIterators()
{
    for( int i=0; i<choices.size(); i++ )
    {
        DecidedQueryCommon::Choice &choice = choices[i];
        if( choice.mode == DecidedQueryCommon::Choice::BEGIN )
        {        
            choice.mode = DecidedQueryCommon::Choice::ITER;
            choice.iter = decisions[i].begin;
        }
    }
}


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Range &r )
{
    ASSERT( r.inclusive || r.begin != r.end )("no empty decisions"); 
    ContainerInterface::iterator it;
    ASSERT( next_decision != decisions.end() ); // run out of decisions? Shouldn't happen now.
    ASSERT( next_choice != choices.end() );
    switch( next_choice->mode )
    {
        case Choice::ITER:
            // We have an iterator already: any decision submitted here must match the previous one
            // so that the iterator remains valid. See range::operator==(). If this cannot be achieved, 
            // then the agent must use IsNextChoiceValid() and if it returns true, call SkipDecision()
            it = next_choice->iter; // Use the iterator that was given to us
            ASSERT( r == *next_decision );
            break;
        
        case Choice::BEGIN:
            // We do not have an iterator: we are able to change the decision
            it = r.begin; // we have been asked to use begin
            *next_decision = r;
            break;
    }
    ASSERT( r.inclusive || it != r.end )("no empty decisions"); 
    ASSERT( it == r.end || *it )("A choice cannot be a nullptr");
    ++next_decision; 
    ++next_choice;
    
    return it;
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( ContainerInterface::iterator begin,
                                                             ContainerInterface::iterator end,
                                                             bool inclusive,
                                                             shared_ptr<ContainerInterface> container )
{
    Range r;
    r.begin = begin;
    r.end = end;
    r.inclusive = inclusive;
    r.container = container;
            
    return RegisterDecision( r );
}
                                                                    
                               
ContainerInterface::iterator DecidedQuery::RegisterDecision( shared_ptr<ContainerInterface> container, bool inclusive )
{
    ASSERT( container );
    return RegisterDecision( container->begin(), container->end(), inclusive, container );
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Sequence<Node> &container, bool inclusive )
{
    auto container_for_query = make_shared< Sequence<Node> >();
    *container_for_query = container;
    ContainerInterface::iterator query_it = RegisterDecision( container_for_query, inclusive );
    //auto p_myit = dynamic_cast<const Sequence<Node>::iterator *>(query_it.GetUnderlyingIterator()); // for #109
    return query_it;
}                                                      


ContainerInterface::iterator DecidedQuery::RegisterDecision( const Collection<Node> &container, bool inclusive )
{
    auto container_for_query = make_shared< Collection<Node> >();
    *container_for_query = container;
    ContainerInterface::iterator query_it = RegisterDecision( container_for_query, inclusive );
    //auto p_myit = dynamic_cast<const Collection<Node>::iterator *>(query_it.GetUnderlyingIterator()); // for #109
    return query_it;
}                                                      


bool DecidedQuery::IsNextChoiceValid() const
{
    return next_choice->mode == Choice::ITER;
}


const DecidedQueryCommon::Range &DecidedQuery::GetNextOldDecision() const
{
    ASSERT( next_decision != decisions.end() )
          ("%d [%p %p ... %p] %p\n", decisions.size(), &decisions[0], &decisions[1], &decisions.back(), &*next_decision);
    return *next_decision;
}


ContainerInterface::iterator DecidedQuery::SkipDecision()
{
    ContainerInterface::iterator it;
    ASSERT( next_decision != decisions.end() ); // run out of decisions? Shouldn't happen now.
    ASSERT( next_choice != choices.end() );
    switch( next_choice->mode )
    {
        case Choice::ITER:
            it = next_choice->iter; // Use the iterator that was given to us
            break;
        
        case Choice::BEGIN:
            it = next_decision->begin; // we have been asked to use begin
            break;
    }
    ASSERT( it == next_decision->end || *it )("A choice cannot be a nullptr");
    ++next_decision; 
    ++next_choice;
    
    return it;
}


void DecidedQuery::CompleteDecisionsWithEmpty()
{
    // This part needed after each DQ
    auto nd = next_decision;
    while( nd != decisions.end() )
    {
        *nd = empty_range;
        ++nd; 
    }    
}


void DecidedQuery::Reset()
{
    normal_links.clear();
	abnormal_links.clear();
    multiplicity_links.clear();
    evaluator = shared_ptr<BooleanEvaluator>();    
    next_decision = decisions.begin();  
    next_choice = choices.begin();  
}


DecidedQuery::Links DecidedQuery::GetAllLinks() const
{
    Links links;
    for( auto link : GetNormalLinks() )
        links.push_back( link );
    for( auto link : GetAbnormalLinks() )
        links.push_back( link );
    for( auto link : GetMultiplicityLinks() )
        links.push_back( link );
    return links;
}


shared_ptr< Collection<Node> > DecidedQuery::empty_container = make_shared< Collection<Node> >();
DecidedQuery::Range DecidedQuery::empty_range { empty_container->begin(), empty_container->end(), false, empty_container };