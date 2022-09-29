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

XLink TeleportAgent::TeleportQueryUnique( const TreeKit &kit, XLink keyer_xlink, bool expect_in_domain ) const
{
    TreePtr<Node> tp_node = RunTeleportQuery( kit, keyer_xlink );
    if( !tp_node )
        return XLink();        
        
    // Uniquify the link against the domain
    return my_scr_engine->UniquifyDomainExtension(tp_node, expect_in_domain);
}                                    


SYM::Over<SYM::BooleanExpression> TeleportAgent::SymbolicNormalLinkedQueryPRed() const                                      
{             
    shared_ptr<PatternQuery> my_pq = GetPatternQuery();         
    PatternLink child_plink = OnlyElementOf( my_pq->GetNormalLinks() );
    
    SYM::Over<SYM::SymbolExpression> keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    SYM::Over<SYM::SymbolExpression> child_expr = MakeOver<SymbolVariable>(child_plink);
    
    return MakeOver<TeleportOperator>( this, keyer_expr ) == child_expr;
}                     


set<XLink> TeleportAgent::ExpandNormalDomain( const TreeKit &kit, const unordered_set<XLink> &keyer_xlinks )
{
    set<XLink> extra_xlinks;
    for( XLink keyer_xlink : keyer_xlinks )
    {
        if( keyer_xlink == XLink::MMAX_Link )
            continue; // MMAX at base never expands domain because then, all child patterns are also MMAX
        if( !IsPreRestrictionMatch(keyer_xlink) )
            continue; // Failed pre-restriction so can't expand domain

        try
        {
            XLink unique_xlink = TeleportQueryUnique( kit, keyer_xlink, false );
            if( unique_xlink )
                extra_xlinks.insert( (XLink)unique_xlink );
        }
        catch( ::Mismatch & ) {}
    }
    return extra_xlinks;
}


void TeleportAgent::Reset()
{
    AgentCommon::Reset();
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


unique_ptr<SymbolResultInterface> TeleportAgent::TeleportOperator::Evaluate( const EvalKit &kit,
                                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    ASSERT( op_results.size()==1 );            
    unique_ptr<SymbolResultInterface> keyer_result = OnlyElementOf(move(op_results));
    if( !keyer_result->IsDefinedAndUnique() )
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    XLink keyer_xlink = keyer_result->GetOnlyXLink();
    XLink unique_xlink = agent->TeleportQueryUnique( *(kit.x_tree_db), keyer_xlink, true );        
    if( unique_xlink )
        return make_unique<SymbolResult>( unique_xlink );
    else 
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
}


Orderable::Result TeleportAgent::TeleportOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                      OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order is unique by owning agent
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
    return agent->GetName() + "(" + keyer->Render() + ")"; 
}


Expression::Precedence TeleportAgent::TeleportOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}


