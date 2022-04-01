#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "expression_analysis.hpp"
#include "result.hpp"
#include "truth_table.hpp"
#include "truth_table_with_predicates.hpp"

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<SymbolExpression> target ) const
{
    shared_ptr<Expression> solution = equation->TrySolveForToEqual( target,
                                                                    make_shared<BooleanConstant>(true) );
    if( !solution )
        return nullptr;
    
    auto sym_solution = dynamic_pointer_cast<SymbolExpression>(solution);
    ASSERT( sym_solution )(solution->Render()); // target is shared_ptr<SymbolicExpression> so should get symbol as solution
    return sym_solution;
}

// -------------------------- TruthTableSolver ----------------------------    

TruthTableSolver::TruthTableSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


void TruthTableSolver::PreSolve()
{
    string s;
    s += "Presolve equation: " + equation->Render() + "\n\n";
    
    auto predicates = PredicateAnalysis::GetPredicates( equation );
    ttwp = make_unique<TruthTableWithPredicates>( predicates, true );

    s += RenderEquationInTermsOfPreds()+"\n";
    
    PopulateInitial();

    s += ttwp->Render( {}, label_var_name, counting_based )+"\n";
    TRACE(s);

    ConstrainUsingDerived();
    
    s = ttwp->Render( {}, label_var_name, counting_based )+"\n";
    TRACE(s);
}


void TruthTableSolver::PopulateInitial()
{
    // Gets them all because they're all true atm
    set<vector<bool>> all_indices = ttwp->GetTruthTable().GetIndicesOfValue(true);
    const SR::SolutionMap sm{};
    const SR::TheKnowledge tk{};
    Expression::EvalKit kit { &sm, &tk }; 
    
    for( vector<bool> indices : all_indices )
    {
        ASSERT( indices.size() == ttwp->GetDegree() );
        vector<shared_ptr<BooleanResult>> vr; // must stay in scope across the Evaluate
        
        for( bool b : indices )
            vr.push_back( make_shared<BooleanResult>(ResultInterface::DEFINED, b) );
        
        for( int j=0; j<ttwp->GetDegree(); j++ )
            for( shared_ptr<PredicateOperator> pred : ttwp->GetPredicateSet(j) )
                pred->SetForceResult( vr[j] );       
            
        shared_ptr<BooleanResultInterface> eval_result = equation->Evaluate(kit);
        
        // Rule out any evaluations that come out false
        if( !eval_result->IsDefinedAndTrue() )
            ttwp->GetTruthTable().Set( indices, false );
    }
}


void TruthTableSolver::ConstrainUsingDerived()
{
    // Get all the extrapolations into maps, keyed by expression equality
    typedef set<int> InitialPredIndices;
    typedef shared_ptr<PredicateOperator> DerivedPred;
    map<DerivedPred, set<InitialPredIndices>, Expression::OrderComparer> derived_pred_to_init_indices;
    map<DerivedPred, set<DerivedPred>, Expression::OrderComparer> derived_pred_to_equal_derived_preds;
    for( int i=0; i<ttwp->GetDegree(); i++ )
    {
        for( int j=0; j<ttwp->GetDegree(); j++ )
        {
            if( i==j )
                continue;

            auto pi = ttwp->GetFrontPredicate(i);
            auto pj = ttwp->GetFrontPredicate(j);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = pi->TryDerive( pj );
            if( pk && !ttwp->PredExists(pk) ) // is this an extrapolation?
            {
                // derived_pred_to_init_indices will unique-ize on equality of pk 
                set<InitialPredIndices> &init_indices_for_k = derived_pred_to_init_indices[pk];
                init_indices_for_k.insert( { i, j } );
                
                set<DerivedPred> &derived_preds_for_k = derived_pred_to_equal_derived_preds[pk];
                derived_preds_for_k.insert( pk );
            }
        }
    }

    // Get them into vectors, which establishes indices (k) for them
    //vector<pair<InitialPredIndices, DerivedPred>> init_indices_and_derived_preds;
    vector<set<DerivedPred>> derived_preds;
    for( auto p : derived_pred_to_init_indices )
    {
        TRACE("Derived predicate ")(p.first->Render())(" appears %d times\n", p.second.size());
        if( p.second.size() > 1 )
        {
            //pair<InitialPredIndices, DerivedPred> ii_and_dp = make_pair(FrontOf(p.second), p.first);
            //init_indices_and_derived_preds.push_back( ii_and_dp );
            
            derived_preds.push_back( derived_pred_to_equal_derived_preds.at(p.first) );
        }
    }
    
    // Policy for extending the truth table. Don't make one with degree more than 10
    // also obviously don't bother if there are no extensions.
    int original_degree = ttwp->GetDegree();
    bool should_expand = derived_preds.size() > 0 &&
                         (original_degree+derived_preds.size() <= 10);
    
    if( should_expand )    
        ttwp->Extend( derived_preds ); 
    
    for( int i=0; i<ttwp->GetDegree(); i++ )
    {
        for( int j=0; j<ttwp->GetDegree(); j++ )
        {
            if( i==j )
                continue;

            auto pi = ttwp->GetFrontPredicate(i);
            auto pj = ttwp->GetFrontPredicate(j);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = pi->TryDerive( pj );
            if( pk ) // There is an extrapolation
            {
                bool is_interpolation = ttwp->PredExists(pk);
                if( is_interpolation ) // The extrapolation is an interpolation
                {
                    int k = ttwp->PredToIndex(pk);
                    // Disallow all combinations that break the implication that Pi ∧ Pj => Pk
                    TRACE("Enforcing interpolation: %s ∧ %s => %s\n", PredicateName(i).c_str(), PredicateName(j).c_str(), PredicateName(k).c_str() );  
                    ttwp->GetTruthTable().SetSlice( {{i, true}, {j, true}, {k, false}}, false );
                }
            }
        }
    }

    if( should_expand )
    {
        set<int> fold_axes;
        for( int k0=0; k0<derived_preds.size(); k0++ )
            fold_axes.insert(original_degree + k0);

        *ttwp = ttwp->GetFolded( fold_axes, false );
    }
}


string TruthTableSolver::PredicateName(int j)
{
    return label_var_name + to_string(j+counting_based);
}


string TruthTableSolver::RenderEquationInTermsOfPreds()
{
    string s;
    vector<shared_ptr<string>> pred_names(ttwp->GetDegree());
    for( int j=0; j<ttwp->GetDegree(); j++ )
    {
        pred_names[j] = make_shared<string>(PredicateName(j));
        for( shared_ptr<PredicateOperator> pred : ttwp->GetPredicateSet(j) )
            pred->SetForceRender( pred_names[j] ); // want to use later...
    }
    s += "We require " + equation->Render() + "\n";
    return s;
}
