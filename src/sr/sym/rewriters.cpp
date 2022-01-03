#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"

#define BOOL_EQUALITY_METHOD

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
            case BooleanResult::TRUE:
                break; // no action required
            case BooleanResult::FALSE:
                ASSERT(false)("Got a FALSE BooleanConstant clause in engine and-rule preprocessing");
                // Of course, there IS a correct thing to do - replace the whole list with a single constant FALSE
                return { MakeLazy<BooleanConstant>(false) }; // ...like this
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
            if( bconst_expr->GetValue()->value == (identity ? BooleanResult::TRUE : BooleanResult::FALSE) )                
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
        return MakeLazy<BooleanConstant>(identity);
    else if( out.size()==1 )
        return OnlyElementOf(out);
    else
        return make_shared<OP>( out );
}

// Explicit template instantiations
template class CreateTidiedOperator<AndOperator>;
template class CreateTidiedOperator<OrOperator>;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<SymbolVariable> target_ ) :
    target( target_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<Equation> equation ) const
{
    if( auto equal_op = dynamic_pointer_cast<EqualOperator>(equation) )
    {
        set<shared_ptr<SymbolExpression>> dep_ops, indep_ops;
        for( shared_ptr<SymbolExpression> op : equal_op->GetSymbolOperands() )
        {
            if( IsIndependent( op ) )
                indep_ops.insert( op );
            else
                dep_ops.insert( op );                  
        }
        
        if( indep_ops.empty() || dep_ops.empty() )
            return nullptr; // need at least one
            
        shared_ptr<SymbolExpression> indep_op = *(indep_ops.begin()); // I believe we could pick any if the equation is solveable
            
        for( shared_ptr<SymbolExpression> dep_op : dep_ops )
        {
            if( dynamic_pointer_cast<SymbolVariable>( dep_op ) ) // TODO should be "can dep_op be solved to match indep_op wrt target"?
                return indep_op;
        }
        
        return nullptr;
    }
#if 1
    else if( auto and_op = dynamic_pointer_cast<AndOperator>(equation) )
    {
        set<shared_ptr<ImplicationOperator>> implies;
        set<shared_ptr<BoolEqualOperator>> bequals;
        for( shared_ptr<BooleanExpression> op : and_op->GetBooleanOperands() )
        {
            if( auto o = dynamic_pointer_cast<ImplicationOperator>(op) )       
                implies.insert(o);
            if( auto o = dynamic_pointer_cast<BoolEqualOperator>(op) )       
                bequals.insert(o);
        }
        if( implies.size()==1 && 
            bequals.size()==1 )
        {
            list< shared_ptr<BooleanExpression> > imply_ops = OnlyElementOf(implies)->GetBooleanOperands();           
            list< shared_ptr<BooleanExpression> > beq_ops = OnlyElementOf(bequals)->GetBooleanOperands(); 
            
            // Ugly bit
            SR::PatternLink beq_plink, imply_plink;
            SR::XLink beq_xlink, imply_xlink;
            if( auto imply_nequal_op = dynamic_pointer_cast<NotEqualOperator>(imply_ops.front() ) )
            {
                if( auto imply_nequal_lop = dynamic_pointer_cast<SymbolVariable>(imply_nequal_op->GetOperands().front() ) )
                    imply_plink = imply_nequal_lop->GetPatternLink();
                if( auto imply_nequal_rop = dynamic_pointer_cast<SymbolConstant>(imply_nequal_op->GetOperands().back() ) )
                    imply_xlink = imply_nequal_rop->GetXLink();
            }
            if( auto beq_equal_op = dynamic_pointer_cast<EqualOperator>(beq_ops.front() ) )
            {
                if( auto beq_equal_lop = dynamic_pointer_cast<SymbolVariable>(beq_equal_op->GetOperands().front() ) )
                    beq_plink = beq_equal_lop->GetPatternLink();
                if( auto beq_equal_rop = dynamic_pointer_cast<SymbolConstant>(beq_equal_op->GetOperands().back() ) )
                    beq_xlink = beq_equal_rop->GetXLink();
            }            

            if( beq_plink && beq_plink==imply_plink &&
                beq_xlink && beq_xlink==imply_xlink )
            {
                // Fronts of imply and beq are negation of each other.
                shared_ptr<SymbolExpression> try_solve_b = TrySolve(beq_ops.back());
                shared_ptr<SymbolExpression> try_solve_c = TrySolve(imply_ops.back());
                if( try_solve_b && try_solve_c )
                {
                    TRACE("Solved an and!!\n");
                    return make_shared<ConditionalOperator>( beq_ops.front(),
                                                             try_solve_b,
                                                             try_solve_c );
                }
            }
        }
    }
#endif
    return nullptr;
}


bool SymSolver::IsIndependent( shared_ptr<Expression> expr ) const
{
    if( auto sv_op = dynamic_pointer_cast<SymbolVariable>(expr) )
        return sv_op->GetPatternLink() != target->GetPatternLink();      

    for( shared_ptr<Expression> op : expr->GetOperands() )
    {
        if( !IsIndependent( op ) )
            return false;
    }

    return true; // all operands are independent of target
}
