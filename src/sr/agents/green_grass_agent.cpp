#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"
#include "sym/primary_expressions.hpp"

using namespace SR;
using namespace SYM;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
    pq->RegisterNormalLink( PatternLink(this, GetThrough()) );
    return pq;
}


bool GreenGrassAgent::ImplHasNLQ() const
{    
    return true;
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


list<shared_ptr<SymbolExpression>> GreenGrassAgent::IsGreenGrassOperator::GetSymbolOperands() const
{
    return { a };
}


shared_ptr<BooleanResult> GreenGrassAgent::IsGreenGrassOperator::Evaluate( const EvalKit &kit,
                                                                           const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResult> ra = OnlyElementOf(op_results);
    if( ra->cat == SymbolResult::UNDEFINED )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    
    if( dirty_grass->count( ra->xlink.GetChildX() ) > 0 ) 
        return make_shared<BooleanResult>( BooleanResult::FALSE );         
    return make_shared<BooleanResult>( BooleanResult::TRUE );
}


Orderable::Result GreenGrassAgent::IsGreenGrassOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const IsGreenGrassOperator *>(candidate);    
    ASSERT(c);

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


string GreenGrassAgent::IsGreenGrassOperator::Render() const
{
    return "IsGreenGrass<" + SSPrintf("%p", dirty_grass)  + ">(" + a->Render() + ")"; 
}


Expression::Precedence GreenGrassAgent::IsGreenGrassOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}
