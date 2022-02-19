#include "set_operators.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- ComplementOperator --------------------------

ComplementOperator::ComplementOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> ComplementOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> ComplementOperator::Evaluate( const EvalKit &kit,
                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    shared_ptr<SymbolSetResult> asr =  SymbolSetResult::Create( ar );
    return asr->GetComplement();
}


string ComplementOperator::Render() const
{
    return "ç" + RenderForMe(a);
}


Expression::Precedence ComplementOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

