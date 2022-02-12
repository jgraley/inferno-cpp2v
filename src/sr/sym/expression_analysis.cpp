#include "common/common.hpp"
#include "common/orderable.hpp"

#include "expression_analysis.hpp"

using namespace SYM;

// -------------------------- ExpressionWalker ----------------------------    

ExpressionWalker::ExpressionWalker( const Lambda &lambda_ ) :
    lambda( lambda_ )
{
}


void ExpressionWalker::operator()(shared_ptr<Expression> expr)
{
    bool cont = lambda( expr );
    if( !cont )
        return;

    list<shared_ptr<Expression>> ops = expr->GetOperands();
    for( shared_ptr<Expression> op : ops )
        operator()(op);
}

// -------------------------- PredicateAnalysis ----------------------------    

void PredicateAnalysis::CheckRegularPredicateForm( shared_ptr<Expression> expr )
{
    ExpressionWalker w( [&](shared_ptr<Expression> expr) -> bool
    {
        auto sym_expr = dynamic_pointer_cast<SymbolExpression>(expr);
        auto bool_expr = dynamic_pointer_cast<BooleanExpression>(expr);
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
        
        // malformed predicate has bool operands
        ASSERT( !(bool_expr && got_sym_ops && got_bool_ops) );           
        
        return true;
    } );

    w(expr);
}


list<shared_ptr<BooleanExpression>> PredicateAnalysis::GetPredicates( shared_ptr<Expression> expr )
{
    list<shared_ptr<BooleanExpression>> preds;
    
    ExpressionWalker w( [&](shared_ptr<Expression> expr) -> bool
    {
        auto bool_expr = dynamic_pointer_cast<BooleanExpression>(expr);
        list<shared_ptr<Expression>> ops = expr->GetOperands();
        bool got_bool_ops = false;
        bool got_sym_ops = false;
        for( shared_ptr<Expression> op : ops )
        {            
            got_bool_ops = got_bool_ops || dynamic_pointer_cast<BooleanExpression>(op);
            got_sym_ops = got_sym_ops || dynamic_pointer_cast<SymbolExpression>(op);
        }

        if( bool_expr && got_sym_ops && !got_bool_ops )
        {
            preds.push_back(bool_expr);
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

