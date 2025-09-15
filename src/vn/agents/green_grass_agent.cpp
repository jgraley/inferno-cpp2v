#include "green_grass_agent.hpp"
#include "../search_replace.hpp" 
#include "../scr_engine.hpp"
#include "link.hpp"
#include "sym/symbol_operators.hpp"
#include "sym/result.hpp"

using namespace VN;
using namespace SYM;

shared_ptr<PatternQuery> GreenGrassAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(GetThrough()) );
    return pq;
}


Lazy<BooleanExpression> GreenGrassAgent::SymbolicAutolocatingQuery(PatternLink keyer_plink) const
{
    auto keyer_expr = MakeLazy<SymbolVariable>(keyer_plink);
    return MakeLazy<IsGreenGrassOperator>(this, keyer_expr);
}


Graphable::NodeBlock GreenGrassAgent::GetGraphBlockInfo() const
{
    // The GreenGrass node appears as a cylinder containing four vertical line characters,
    // like this: ||||. These are meant to represent the blades of grass. It was late and I was
    // tired.
    NodeBlock block;
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
    block.item_blocks = { { "through", 
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
                                                                           list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
	(void)kit;
    ASSERT( op_results.size()==1 );        
    unique_ptr<SymbolicResult> ra = SoloElementOf(move(op_results));
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );    
    TreePtr<Node> tp = ra->GetOnlyXLink().GetChildTreePtr();
    
    bool green = !tp->WasInventedDuringCurrentStep();
        
    return make_unique<BooleanResult>( green );         
}


Orderable::Diff GreenGrassAgent::IsGreenGrassOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                                                  OrderProperty order_property ) const 
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
    // Agents aren't comparable, so value of operator is identiy of agent
    return Node::Compare3WayIdentity( *agent->GetPatternPtr(), *r.agent->GetPatternPtr() );
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
