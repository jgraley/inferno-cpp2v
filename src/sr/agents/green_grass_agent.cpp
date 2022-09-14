#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"

using namespace SR;
using namespace SYM;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


Over<BooleanExpression> GreenGrassAgent::SymbolicColocatedQuery() const
{
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    return MakeOver<IsGreenGrassOperator>(this, keyer_expr);
}


Graphable::Block GreenGrassAgent::GetGraphBlockInfo() const
{
	// The GreenGrass node appears as a cylinder containing four vertical line characters,
	// like this: ||||. These are meant to represent the blades of grass. It was late and I was
	// tired.
    Block block;
	block.bold = true;
    block.title = "GreenGrass";
	block.symbol = "||||";
	block.shape = "cylinder";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>( GetThrough()->get() ), 
              list<string>{},
              list<string>{},
              phase,
              GetThrough() );
    block.sub_blocks = { { "through", 
                           "", 
                           true,
                           { link } } };
    return block;
}


GreenGrassAgent::IsGreenGrassOperator::IsGreenGrassOperator( const GreenGrassAgent *agent_,
                                                             shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    agent( agent_ )
{    
}                                                


shared_ptr<PredicateOperator> GreenGrassAgent::IsGreenGrassOperator::Clone() const
{
    return make_shared<IsGreenGrassOperator>( agent, a );
}
    

list<shared_ptr<SymbolExpression> *> GreenGrassAgent::IsGreenGrassOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> GreenGrassAgent::IsGreenGrassOperator::Evaluate( const EvalKit &kit,
                                                                                    list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    unique_ptr<SymbolResultInterface> ra = OnlyElementOf(move(op_results));
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );
    
    bool green = !( agent->my_scr_engine->IsDirtyGrass( ra->GetOnlyXLink().GetChildX() ) ); 
    return make_unique<BooleanResult>( green );         
}


Orderable::Result GreenGrassAgent::IsGreenGrassOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses agent address to ensure different agents compare different
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


string GreenGrassAgent::IsGreenGrassOperator::RenderNF() const
{
	// TODO put the agent trace in the <>
    return "IsGreenGrass<>(" + a->Render() + ")"; 
}


Expression::Precedence GreenGrassAgent::IsGreenGrassOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}
