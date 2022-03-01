#include "common/common.hpp"
#include "common/orderable.hpp"
#include "predicate_operators.hpp"

#include "expression_analysis.hpp"

using namespace SYM;

// -------------------------- ExpressionWalker ----------------------------    

ExpressionWalker::ExpressionWalker( bool include_root_, const Lambda &lambda_ ) :
    include_root( include_root_ ),
    lambda( lambda_ )
{
}


void ExpressionWalker::operator()( shared_ptr<Expression> expr, bool is_root )
{
    if( include_root || !is_root )
    {
        bool cont = lambda( expr );
        if( !cont )
            return;
    }

    list<shared_ptr<Expression>> ops = expr->GetOperands();
    for( shared_ptr<Expression> op : ops )
        operator()(op, false);
}

// -------------------------- PredicateAnalysis ----------------------------    

void PredicateAnalysis::CheckRegularPredicateForm( shared_ptr<Expression> expr )
{
    ExpressionWalker w( true, [&](shared_ptr<Expression> expr) -> bool
    {
        auto sym_expr = dynamic_pointer_cast<SymbolExpression>(expr);
        auto bool_expr = dynamic_pointer_cast<BooleanExpression>(expr);
        auto pred_expr = dynamic_pointer_cast<PredicateOperator>(expr);
        list<shared_ptr<Expression>> ops = expr->GetOperands();
        bool got_bool_ops = false;
        bool got_sym_ops = false;
        for( shared_ptr<Expression> op : ops )
        {            
            got_bool_ops = got_bool_ops || dynamic_pointer_cast<BooleanExpression>(op);
            got_sym_ops = got_sym_ops || dynamic_pointer_cast<SymbolExpression>(op);
        }
        
        // no boolean operand to an expression that returns a symbol (anti-predicate) 
        ASSERT( !(sym_expr && got_bool_ops) );           
        
        if( pred_expr ) // Check validity of predicates
        {
            ASSERT( bool_expr ); // Predicates must return boolean
            ASSERT( !got_bool_ops ); // and can only have symbolic operands
            ASSERT( got_sym_ops ); // would be odd if no ops - I can only think of constants, which we can leave in the bool region
            
            CheckNoPredicatesUnder(pred_expr); // no predicates underneath
            return false; // No need to recurse since we just did that
        }
        
        return true;
    } );

    w(expr);
}


void PredicateAnalysis::CheckNoPredicatesUnder( shared_ptr<Expression> expr )
{
    ExpressionWalker w( false, [&](shared_ptr<Expression> expr) -> bool
    {
        ASSERT( !dynamic_pointer_cast<PredicateOperator>(expr) );
        return true;
    } );

    w(expr);
}


vector<shared_ptr<PredicateOperator>> PredicateAnalysis::GetPredicates( shared_ptr<Expression> expr )
{
    vector<shared_ptr<PredicateOperator>> preds;
    
    ExpressionWalker w( true, [&](shared_ptr<Expression> expr) -> bool
    {
        if( auto pred_expr = dynamic_pointer_cast<PredicateOperator>(expr) )
        {
            preds.push_back(pred_expr);
            return false; // no need to recurse into predicate
        }
        else
        {
            return true; // keep looking
        }
    } );

    w(expr);
    return preds;
}

