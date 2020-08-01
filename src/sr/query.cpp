#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

using namespace SR;

void PatternQueryResult::AddLink( bool abnormal, Agent *a )
{
    Link b;
    b.abnormal = abnormal;
    b.agent = a;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    TRACE("Remembering block %d ", links.size())(*a)(abnormal?" abnormal":" normal")("\n");
    links.push_back( b );
}


void AgentQuery::AddLink( bool abnormal, Agent *a, const TreePtrInterface *px )
{
    Link b;
    b.abnormal = abnormal;
    b.agent = a;
    b.px = px;
    b.local_x = TreePtr<Node>();
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    TRACE("Remembering block %d ", links.size())(*a)(" -> ")(**px)(abnormal?" abnormal":" normal")("\n");
    links.push_back( b );
}


void AgentQuery::AddLocalLink( bool abnormal, Agent *a, TreePtr<Node> x )
{
    ASSERT(x);
    Link b;
    b.abnormal = abnormal;
    b.agent = a;
    b.px = NULL;    
    b.local_x = x;
    
    // For debugging
    b.whodat = __builtin_extract_return_addr (__builtin_return_address (0));
    
    TRACE("Remembering local block %d ", links.size())(*a)(" -> ")(*x)(abnormal?" abnormal":" normal")("\n");
    links.push_back( b );
}


void PatternQueryResult::AddEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	


void AgentQuery::AddEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	
                         
                                        
bool SR::operator<(const SR::AgentQuery::Link &l0, const SR::AgentQuery::Link &l1)
{
    if( l0.abnormal != l1.abnormal )
        return (int)l0.abnormal < (int)l1.abnormal;
    if( l0.agent != l1.agent )
        return l0.agent < l1.agent;
    if( l0.px != l1.px )
        return l0.px < l1.px;    
    if( l0.local_x != l1.local_x )
        return l0.local_x < l1.local_x;    
        
    return false; // equal
}


void AgentQuery::InvalidateBack()
{
    // TODO may not need all thes preconditions
    ASSERT( !choices.empty() );
    ASSERT( !decisions.empty() );
    ASSERT( choices.size() == decisions.size() ); 
    choices.pop_back();
    decisions.pop_back();    
}


void AgentQuery::SetBackChoice( ContainerInterface::iterator newc )
{
    choices.back() = newc;
}


void AgentQuery::PushBackChoice( ContainerInterface::iterator newc )
{
    choices.push_back(newc);
}


ContainerInterface::iterator AgentQuery::AddDecision( const Range &r )
{
 
    ASSERT( r.inclusive || r.begin != r.end )("no empty decisions");
    ContainerInterface::iterator it;
    if( next_decision == decisions.end() ) // run out of decisions?
    {
        it = r.begin; // No choice was given to us so assume first one
        ASSERT( it == r.end || *it )("A choice cannot be a nullptr");
        decisions.push_back(r); // this will be a new decision
        next_decision = decisions.end(); // beware iterator invalidation
    }
    else
    {
        ASSERT( next_choice != choices.end() );
        it = *next_choice; // Use the choice that was given to us
        ASSERT( r.inclusive || it != r.end )("A choice can only be end if the decision is inclusive");
        ASSERT( it == r.end || *it )("A choice cannot be a nullptr");
        *next_decision = r; // overwrite TODO they should be identical!
        ++next_decision; 
        ++next_choice;
    }    
    
    return it;
}                                                      


ContainerInterface::iterator AgentQuery::AddDecision( ContainerInterface::iterator begin,
                                                      ContainerInterface::iterator end,
                                                      bool inclusive,
                                                      shared_ptr<ContainerInterface> container )
{
    Range r;
    r.begin = begin;
    r.end = end;
    r.inclusive = inclusive;
    r.container = container;
            
    return AddDecision( r );
}
                                                                    
                               
ContainerInterface::iterator AgentQuery::AddDecision( shared_ptr<ContainerInterface> container, bool inclusive )
{
    ASSERT( container );
    return AddDecision( container->begin(), container->end(), inclusive, container );
}                                                      


/* Need to fix OOStd to permit the assignment  #53
ContainerInterface::iterator AgentQuery::AddDecision( const ContainerInterface &container, bool inclusive )
{
    auto container_for_query = make_shared< Collection<Node> >();
    *container_for_query = container;
    return AddDecision( container_for_query, inclusive );
}                                                      
*/

bool AgentQuery::IsAlreadyGotNextOldDecision()
{
    return next_decision != decisions.end();
}


const Conjecture::Range &AgentQuery::GetNextOldDecision()
{
    ASSERT( IsAlreadyGotNextOldDecision() );
    return *next_decision;
}


ContainerInterface::iterator AgentQuery::AddNextOldDecision()
{
    ASSERT( next_choice != choices.end() );
    ContainerInterface::iterator it = *next_choice; // Use the choice that was given to us
    ++next_decision; 
    ++next_choice;
    return it;
}


void AgentQuery::Reset()
{
    links.clear();
    evaluator = shared_ptr<BooleanEvaluator>();    
    next_decision = decisions.begin();  
    next_choice = choices.begin();  
}