#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"
#include <inttypes.h>

using namespace SR;

void EnsureOnStack( const TreePtrInterface *ppattern )
{
    ASSERT( (((uint64_t)ppattern & 0x7fff00000000ULL) != 0x7fff00000000ULL) )
          ("Supplied agent link seems like it's probably on the stack, usually a bad sign\n");
}


void PatternQuery::RegisterDecision( bool inclusive )
{
    Decision d;
    d.inclusive = inclusive;
    decisions.push_back(d);
}

 
void PatternQuery::RegisterNormalLink( const TreePtrInterface *ppattern )
{
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );        
}


void PatternQuery::RegisterAbnormalLink( const TreePtrInterface *ppattern )
{
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    abnormal_links.push_back( b );       
}


void PatternQuery::RegisterMultiplicityLink( const TreePtrInterface *ppattern )
{
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    multiplicity_links.push_back( b );       
}


DecidedQuery::DecidedQuery( shared_ptr<const PatternQuery> pq ) :
    decisions( pq->GetDecisions()->size() ),
    next_decision( decisions.begin() ), 
    choices( pq->GetDecisions()->size() ),
    next_choice( choices.begin() ) 
{
    CompleteDecisionsWithEmpty();
}


void DecidedQuery::Start()
{
    for( Range &r : decisions )
        r = empty_range;
}


void DecidedQuery::RegisterNormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x )
{
    ASSERT(x);
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    b->x = x;
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );        
}


void DecidedQuery::RegisterAbnormalLink( const TreePtrInterface *ppattern, TreePtr<Node> x )
{
    ASSERT(x);
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    b->x = x;
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    abnormal_links.push_back( b );
}


void DecidedQuery::RegisterMultiplicityLink( const TreePtrInterface *ppattern, TreePtr<SubContainer> x )
{
    ASSERT(x);
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    b->x = x;
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));

    multiplicity_links.push_back( b );
}


void DecidedQuery::RegisterAlwaysMatchingLink( const TreePtrInterface *ppattern )
{
    auto b = make_shared<Link>();
    EnsureOnStack( ppattern );
    TreePtr<Node> npattern(*ppattern);
    b->agent = Agent::AsAgent(npattern);
    // Supply the pattern as x. Pattern are usually not valid x nodes
    // (because can have NULL pointers) but there's logic in 
    // the AndRuleEngine to early-out in this case. 
    b->x = *ppattern;
    
    // For debugging
    b->whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );      
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


shared_ptr< Collection<Node> > DecidedQuery::empty_container = make_shared< Collection<Node> >();
DecidedQuery::Range DecidedQuery::empty_range { empty_container->begin(), empty_container->end(), false, empty_container };