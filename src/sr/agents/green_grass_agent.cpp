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
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


Over<BooleanExpression> GreenGrassAgent::SymbolicColocatedQuery() const
{
    auto keyer_expr = MakeOver<SymbolVariable>(keyer_plink);
    const set< TreePtr<Node> > *dirty_grass = &master_scr_engine->GetOverallMaster()->dirty_grass;    
    return MakeOver<IsGreenGrassOperator>(dirty_grass, keyer_expr);
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
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>( GetThrough()->get()), 
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


GreenGrassAgent::IsGreenGrassOperator::IsGreenGrassOperator( const set< TreePtr<Node> > *dirty_grass_,
                                                             shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    dirty_grass( dirty_grass_ )
{    
}                                                


GreenGrassAgent::IsGreenGrassOperator *GreenGrassAgent::IsGreenGrassOperator::Clone() const
{
    return new IsGreenGrassOperator( dirty_grass, a );
}
    

list<shared_ptr<SymbolExpression> *> GreenGrassAgent::IsGreenGrassOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> GreenGrassAgent::IsGreenGrassOperator::Evaluate( const EvalKit &kit,
                                                                                    const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResultInterface> ra = OnlyElementOf(op_results);
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );
    
    bool res = ( dirty_grass->count( ra->GetOnlyXLink().GetChildX() ) == 0 ); 
    return make_unique<BooleanResult>( res );         
}


Orderable::Result GreenGrassAgent::IsGreenGrassOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses address to ensure different dirty_grass sets compare differently
        r = (int)(dirty_grass > c->dirty_grass) - (int)(dirty_grass < c->dirty_grass);
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
    return "IsGreenGrass<" + SSPrintf("%p", dirty_grass)  + ">(" + a->Render() + ")"; 
}


Expression::Precedence GreenGrassAgent::IsGreenGrassOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}
