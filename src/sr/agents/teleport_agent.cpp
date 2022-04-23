#include "teleport_agent.hpp"

#include "link.hpp"

// Temporary
#include "tree/cpptree.hpp"

#include "transform_of_agent.hpp"
#include "sym/boolean_operators.hpp"
#include "sym/predicate_operators.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"

#include <stdexcept>

using namespace SR;
using namespace SYM;

//---------------------------------- TeleportAgent ------------------------------------    

LocatedLink TeleportAgent::TeleportUniqueAndCache( XLink keyer_xlink ) const
{
    auto op = [&](XLink keyer_xlink) -> LocatedLink
    {
        LocatedLink tp_link = RunTeleportQuery( keyer_xlink );
        if( !tp_link )
            return tp_link;
        
        // We will uniquify the link against the domain and then cache it against keyer_xlink
        LocatedLink ude_link( (PatternLink)tp_link, master_scr_engine->UniquifyDomainExtension(tp_link) ); 
                   
        return ude_link;
    };
    
    return cache( keyer_xlink, op );
}                                    


SYM::Over<SYM::BooleanExpression> TeleportAgent::SymbolicNormalLinkedQueryPRed() const                                      
{             
    shared_ptr<PatternQuery> my_pq = GetPatternQuery();         
    PatternLink child_plink = OnlyElementOf( my_pq->GetNormalLinks() );
    
    SYM::Over<SYM::SymbolExpression> keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    SYM::Over<SYM::SymbolExpression> child_expr = MakeOver<SymbolVariable>(child_plink);
    
    return MakeOver<TeleportOperator>( this, keyer_expr ) == child_expr;
}                     


set<XLink> TeleportAgent::ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks )
{
    set<XLink> extra_xlinks;
    for( XLink keyer_xlink : keyer_xlinks )
    {
        if( keyer_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because all child patterns are also MMAX
        if( !IsPreRestrictionMatch(keyer_xlink) )
            continue; // Failed pre-restriction so can't expand domain

        try
        {
            LocatedLink cached_link = TeleportUniqueAndCache( keyer_xlink );
            if( cached_link )
                extra_xlinks.insert( (XLink)cached_link );
        }
        catch( ::Mismatch & ) {}
    }
    return extra_xlinks;
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
    cache.Reset();
}


TeleportAgent::TeleportOperator::TeleportOperator( const TeleportAgent *agent_,
                                                   shared_ptr<SymbolExpression> keyer_ ) :
    agent( agent_ ),
    keyer( keyer_ )
{    
}                                                


list<shared_ptr<SymbolExpression>> TeleportAgent::TeleportOperator::GetSymbolOperands() const
{
    return { keyer };
}


shared_ptr<SymbolResultInterface> TeleportAgent::TeleportOperator::Evaluate( const EvalKit &kit,
                                                                             const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResultInterface> keyer_result = OnlyElementOf(op_results);
    if( !keyer_result->IsDefinedAndUnique() )
        return make_shared<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    XLink keyer_xlink = keyer_result->GetOnlyXLink();
    LocatedLink cached_link = agent->TeleportUniqueAndCache( keyer_xlink );        
    if( (XLink)cached_link )
        return make_shared<SymbolResult>( (XLink)cached_link );
    else 
        return make_shared<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
}


Orderable::Result TeleportAgent::TeleportOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                      OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses address to ensure different dirty_grass sets compare differently
        r = (int)(agent > c->agent) - (int)(agent < c->agent);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
        // Repeatable ordering stops after name check since address compare is not repeatable
        r = Orderable::EQUAL;
        break;
    }
    return r;
}  


string TeleportAgent::TeleportOperator::Render() const
{
    return "Teleport<" + agent->GetName() + ">(" + keyer->Render() + ")"; 
}


Expression::Precedence TeleportAgent::TeleportOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}


