#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

using namespace SR;


void PatternQuery::RegisterDecision( bool inclusive )
{
    Decision d;
    d.inclusive = inclusive;
    decisions.push_back(d);
}


void PatternQuery::RegisterNormalLink( Agent *a )
{
    Link b;
    b.agent = a;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );        
}


void PatternQuery::RegisterAbnormalLink( Agent *a )
{
    Link b;
    b.agent = a;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    abnormal_links.push_back( b );       
}


void PatternQuery::RegisterMultiplicityLink( Agent *a )
{
    Link b;
    b.agent = a;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    multiplicity_links.push_back( b );       
}


DecidedQuery::DecidedQuery(const PatternQuery &pq) :
    decisions( pq.GetDecisions()->size() ),
    next_decision( decisions.begin() ), 
    choices( pq.GetDecisions()->size() ),
    next_choice( choices.begin() ) 
{
    CompleteDecisionsWithEmpty();
}


void DecidedQuery::RegisterNormalLink( Agent *a, const TreePtrInterface *px )
{
    Link b;
    b.agent = a;
    b.px = px;
    b.local_x = TreePtr<Node>();
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );        
}


void DecidedQuery::RegisterAbnormalLink( Agent *a, const TreePtrInterface *px )
{
    Link b;
    b.agent = a;
    b.px = px;
    b.local_x = TreePtr<Node>();
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    abnormal_links.push_back( b );     
}


void DecidedQuery::RegisterMultiplicityLink( Agent *a, const TreePtrInterface *px )
{
    Link b;
    b.agent = a;
    b.px = px;
    b.local_x = TreePtr<Node>();
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    multiplicity_links.push_back( b );     
}


void DecidedQuery::RegisterLocalNormalLink( Agent *a, TreePtr<Node> x )
{
    ASSERT(x);
    Link b;
    b.agent = a;
    b.px = NULL;    
    b.local_x = x;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    normal_links.push_back( b );        
}


void DecidedQuery::RegisterLocalAbnormalLink( Agent *a, TreePtr<Node> x )
{
    ASSERT(x);
    Link b;
    b.agent = a;
    b.px = NULL;    
    b.local_x = x;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    abnormal_links.push_back( b );
}


void DecidedQuery::RegisterLocalMultiplicityLink( Agent *a, TreePtr<SubContainer> x )
{
    ASSERT(x);
    Link b;
    b.agent = a;
    b.px = NULL;    
    b.local_x = x;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));

    multiplicity_links.push_back( b );
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
                         
                                        
bool SR::operator<(const SR::DecidedQuery::Link &l0, const SR::DecidedQuery::Link &l1)
{
    if( l0.agent != l1.agent )
        return l0.agent < l1.agent;
    if( l0.px != l1.px )
        return l0.px < l1.px;    
    if( l0.local_x != l1.local_x )
        return l0.local_x < l1.local_x;    
        
    return false; // equal
}


void DecidedQuery::InvalidateBack()
{
    // TODO may not need all thes preconditions
    ASSERT( !choices.empty() );
    ASSERT( !decisions.empty() );
    //ASSERT( choices.size() == decisions.size() ); 
    if( next_decision-decisions.begin() == choices.size() ) // Note: possibly always true
        --next_decision; // Force agent to regenerate decision
  
    //decisions.pop_back();    
    choices.pop_back();
    
}


void DecidedQuery::SetBackChoice( Choice newc )
{
    choices.back() = newc;
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
            it = next_choice->iter; // Use the iterator that was given to us
            break;
        
        case Choice::BEGIN:
            it = r.begin; // we have been asked to use begin
            break;
    }
    ASSERT( r.inclusive || it != r.end )("no empty decisions"); 
    ASSERT( it == r.end || *it )("A choice cannot be a nullptr");
    *next_decision = r;
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


/* Need to fix OOStd to permit the assignment  #53
ContainerInterface::iterator DecidedQuery::RegisterDecision( const ContainerInterface &container, bool inclusive )
{
    auto container_for_query = make_shared< Collection<Node> >();
    *container_for_query = container;
    return RegisterDecision( container_for_query, inclusive );
}                                                      
*/

const DecidedQueryCommon::Range &DecidedQuery::GetNextOldDecision() const
{
    ASSERT( next_decision != decisions.end() )
          ("%d [%p %p ... %p] %p\n", decisions.size(), &decisions[0], &decisions[1], &decisions.back(), &*next_decision);
    return *next_decision;
}


DecidedQuery::Ranges::iterator DecidedQuery::GetNextDecisionIterator() const
{
    return next_decision;  // really an index TODO go back to iterator when we're sure the vector won't be relocated
}


void DecidedQuery::CompleteDecisionsWithEmpty()
{
    Range r;
    r.begin = empty_container->begin();
    r.end = empty_container->end();
    r.inclusive = false;
    r.container = empty_container;
    
    // This part needed after each DQ
    auto nd = next_decision;
    while( nd != decisions.end() )
    {
        *nd = r;
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
