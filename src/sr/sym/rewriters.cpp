#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

BooleanExpressionSet PreprocessForEngine::operator()( BooleanExpressionSet in ) const
{
    BooleanExpressionSet l1;
    for( auto bexpr : in )
    {
        SplitAnds( l1, bexpr );
    }

    BooleanExpressionSet l2;
    for( auto bexpr : l1 )
    {
        if( auto bool_const_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            shared_ptr<BooleanResult> r = bool_const_expr->GetValue();
            switch(r->value)
            {
            case BooleanResult::Certainty::TRUE:
                break; // no action required
            case BooleanResult::Certainty::FALSE:
                ASSERT(false)("Got a FALSE BooleanConstant clause in engine and-rule preprocessing");
                // Of course, there IS a correct thing to do - replace the whole list with a single constant FALSE
                return { MakeOver<BooleanConstant>(false) }; // ...like this
            }
        }
        else
        {
            l2.insert( bexpr );
        }
    }
    
    return l2;
}


void PreprocessForEngine::SplitAnds( BooleanExpressionSet &split, 
                                     shared_ptr<BooleanExpression> original ) const
{
    if( auto and_expr = dynamic_pointer_cast<AndOperator>(original) )
    {
        for( shared_ptr<BooleanExpression> sub_expr : and_expr->GetBooleanOperands() )
            SplitAnds( split, sub_expr );
    }   
    else
    {
        split.insert( original );
    }
}

// ------------------------- CreateTidiedOperator --------------------------

template<typename OP>
CreateTidiedOperator<OP>::CreateTidiedOperator( bool identity_ ) :
    identity(identity_)
{
}


template<typename OP>
shared_ptr<BooleanExpression> CreateTidiedOperator<OP>::operator()( list< shared_ptr<BooleanExpression> > in ) const
{
    list< shared_ptr<BooleanExpression> > out;
    for( auto bexpr : in )
    {
        // Handle finding our own opeator as an operand - expand one level (non-recursive)
        if( auto op_expr = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
        {
            for( shared_ptr<Expression> sub_expr : op_expr->GetOperands() )
                out.push_back( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
        }   
        else if( auto bconst_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            // Handle finding constant booleans relevent to the operator
            if( bconst_expr->GetValue()->value == (identity ? BooleanResult::Certainty::TRUE : BooleanResult::Certainty::FALSE) )                
            {
                // drop the clause - has no effect
            }
            else
            {
                // dominates
                out = { bconst_expr };
                break;
            }                    
        }
        else
        {
            out.push_back( bexpr );
        }
    }
    
    // By cases: empty get identity, singlur gets returned, otherwise actually generate the operator
    if( out.empty() )
        return MakeOver<BooleanConstant>(identity);
    else if( out.size()==1 )
        return OnlyElementOf(out);
    else
        return make_shared<OP>( out );
}

// Explicit template instantiations
template class CreateTidiedOperator<AndOperator>;
template class CreateTidiedOperator<OrOperator>;
