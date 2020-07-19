#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "agent.hpp"

using namespace SR;

void PatternQueryResult::AddLink( bool abnormal, Agent *a )
{
    Block b;
    b.abnormal = abnormal;
    b.agent = a;
    TRACE("Remembering block %d ", blocks.size())(*a)(abnormal?" abnormal":" normal")("\n");
    blocks.push_back( b );
}


void DecidedQueryResult::AddLink( bool abnormal, Agent *a, const TreePtrInterface *px )
{
    Block b;
    b.is_link = true;
    b.abnormal = abnormal;
    b.agent = a;
    b.px = px;
    b.local_x = TreePtr<Node>();
    b.is_decision = false;
    TRACE("Remembering block %d ", blocks.size())(*a)(" -> ")(**px)(abnormal?" abnormal":" normal")("\n");
    blocks.push_back( b );
}


void DecidedQueryResult::AddLocalLink( bool abnormal, Agent *a, TreePtr<Node> x )
{
    ASSERT(x);
    Block b;
    b.is_link = true;
    b.abnormal = abnormal;
    b.agent = a;
    b.px = NULL;    
    b.local_x = x;
    b.is_decision = false;
    TRACE("Remembering local block %d ", blocks.size())(*a)(" -> ")(*x)(abnormal?" abnormal":" normal")("\n");
    blocks.push_back( b );
}


void PatternQueryResult::AddEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	


void DecidedQueryResult::AddEvaluator( shared_ptr<BooleanEvaluator> e )
{
	ASSERT( !evaluator ); // should not register more than one
	evaluator = e;
}	


ContainerInterface::iterator DecidedQueryResult::AddDecision( ContainerInterface::iterator begin,
                                                              ContainerInterface::iterator end,
                                                              bool inclusive,
                                                              const Choices &choices,
                                                              shared_ptr<ContainerInterface> container )
{
    ASSERT( inclusive || begin != end )("no empty decisions");
    ContainerInterface::iterator it;
    if( decision_count >= choices.size() )
    {
        it = begin; // No choice was given to us so assume first one
        ASSERT( it == end || *it );
    }
    else
    {
        it = choices[decision_count]; // Use and consume the choice that was given to us
        ASSERT( inclusive || it != end );
        ASSERT( it == end || *it );
    }    
    
    ASSERT(begin==end || *begin);
    
    Range r;
    r.begin = begin;
    r.end = end;
    r.inclusive = inclusive;
    r.container = container;
    Block b;
    b.is_link = false;
    b.is_decision = true;
    b.decision = r;    
    blocks.push_back( b );
    decision_count++;
        
    return it;
}
                                                                    
                                        
void DecidedQueryResult::AddLocalMatch( bool lm )
{
    local_match = lm;
}                                    
                                        
                                        
bool SR::operator<(const SR::DecidedQueryResult::Block &l0, const SR::DecidedQueryResult::Block &l1)
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


void AgentQuery::SetDQR( const DecidedQueryResult &dqr )
{
    DecidedQueryResult::operator=( dqr );

    // Feed the decisions info in the blocks structure back to the conjecture
    decisions.clear();
    for( const DecidedQueryResult::Block &b : dqr.GetBlocks() )
        if( b.is_decision ) 
            decisions.push_back( b.decision );
}
