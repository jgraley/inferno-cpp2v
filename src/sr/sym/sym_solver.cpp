#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "expression_analysis.hpp"
#include "result.hpp"
#include "truth_table.hpp"

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
    
    predicates = PredicateAnalysis::GetPredicates( equation );
    s += RenderPredicatesAndPredEquation()+"\n";

    truth_table = make_unique<TruthTable>( predicates.size(), true );
    
    PopulateInitial();

    s += truth_table->Render( {}, label_var_name, counting_based )+"\n";
    TRACE(s);

    ConstrainUsingDerived();
    
    s = truth_table->Render( {}, label_var_name, counting_based )+"\n";
    TRACE(s);
}


void TruthTableSolver::PopulateInitial()
{
    // Gets them all because they're all true atm
    set<vector<bool>> all_indices = truth_table->GetIndicesOfValue(true);
    const SR::SolutionMap sm{};
    const SR::TheKnowledge tk{};
    Expression::EvalKit kit { &sm, &tk }; 
    
    for( vector<bool> indices : all_indices )
    {
        ASSERT( indices.size() == predicates.size() );
        vector<shared_ptr<BooleanResult>> vr; // must stay in scope across the Evaluate
        
        for( bool b : indices )
            vr.push_back( make_shared<BooleanResult>(ResultInterface::DEFINED, b) );
        
        for( int j=0; j<predicates.size(); j++ )
            for( shared_ptr<PredicateOperator> pred : predicates[j] )
                pred->SetForceResult( vr[j] );       
            
        shared_ptr<BooleanResultInterface> eval_result = equation->Evaluate(kit);
        
        // Rule out any evaluations that come out false
        if( !eval_result->IsDefinedAndTrue() )
            truth_table->Set( indices, false );
    }
}


void TruthTableSolver::ConstrainUsingDerived()
{
    // Make a map from predicate to index
    map<shared_ptr<PredicateOperator>, int, Expression::OrderComparer> pred_to_index;
    for( int i=0; i<predicates.size(); i++ )
        pred_to_index[FrontOf(predicates[i])] = i;

    // Get all the extrapolations into maps, keyed by expression equality
    typedef set<int> InitialPredIndices;
    typedef shared_ptr<PredicateOperator> DerivedPred;
    map<DerivedPred, set<InitialPredIndices>, Expression::OrderComparer> derived_pred_to_init_indices;
    map<DerivedPred, set<DerivedPred>, Expression::OrderComparer> derived_pred_to_equal_derived_preds;
    for( int i=0; i<predicates.size(); i++ )
    {
        for( int j=0; j<predicates.size(); j++ )
        {
            if( i==j )
                continue;

            auto pi = FrontOf(predicates[i]);
            auto pj = FrontOf(predicates[j]);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = pi->TryDerive( pj );
            if( pk && pred_to_index.count(pk)==0 ) // is this an extrapolation?
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
    vector<pair<InitialPredIndices, DerivedPred>> init_indices_and_derived_preds;
    vector<set<DerivedPred>> derived_preds;
    for( auto p : derived_pred_to_init_indices )
    {
        TRACE("Derived predicate ")(p.first->Render())(" appears %d times\n", p.second.size());
        if( p.second.size() > 1 )
        {
            pair<InitialPredIndices, DerivedPred> ii_and_dp = make_pair(FrontOf(p.second), p.first);
            init_indices_and_derived_preds.push_back( ii_and_dp );
            
            derived_preds.push_back( derived_pred_to_equal_derived_preds.at(p.first) );
        }
    }
    
    // Policy for extending the truth table. Don't make one with degree more than 10
    // also obviously don't bother if there are no extensions.
    bool should_expand = derived_preds.size() > 0 &&
                         (predicates.size()+derived_preds.size() <= 10);
    
    vector<set<shared_ptr<PredicateOperator>>> extended_preds = predicates;
    if( should_expand )
    {
        TRACE("Extending truth table from %d by %d\n", predicates.size(), derived_preds.size());
        truth_table->Extend( predicates.size() + derived_preds.size() ); 
        extended_preds = extended_preds + derived_preds;
    }
    
    pred_to_index.clear();
    for( int i=0; i<extended_preds.size(); i++ )
        pred_to_index[FrontOf(extended_preds[i])] = i;
    for( int i=0; i<extended_preds.size(); i++ )
    {
        for( int j=0; j<extended_preds.size(); j++ )
        {
            if( i==j )
                continue;

            auto pi = FrontOf(extended_preds[i]);
            auto pj = FrontOf(extended_preds[j]);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = pi->TryDerive( pj );
            if( pk ) // There is an extrapolation
            {
                bool is_interpolation = (pred_to_index.count(pk)==1);
                if( is_interpolation ) // The extrapolation is an interpolation
                {
                    int k = pred_to_index.at(pk);
                    // Disallow all combinations that break the implication that Pi ∧ Pj => Pk
                    TRACE("Enforcing interpolation: %s ∧ %s => %s\n", PredicateName(i).c_str(), PredicateName(j).c_str(), PredicateName(k).c_str() );  
                    truth_table->SetSlice( {{i, true}, {j, true}, {k, false}}, false );
                }
            }
        }
    }

    if( should_expand )
    {
        set<int> fold_axes;
        for( int k0=0; k0<init_indices_and_derived_preds.size(); k0++ )
        {
            int k = predicates.size() + k0;
            fold_axes.insert(k);
        }
        *truth_table = truth_table->GetFolded( fold_axes, false );
    }
}



string TruthTableSolver::PredicateName(int j)
{
    return label_var_name + to_string(j+counting_based);
}


string TruthTableSolver::RenderPredicatesAndPredEquation()
{
    string s;
    vector<shared_ptr<string>> pred_names(predicates.size());
    for( int j=0; j<predicates.size(); j++ )
    {
        string name = PredicateName(j);
        s += name + " := " + (*(predicates[j].begin()))->Render() + "\n";

        pred_names[j] = make_shared<string>(name);
        for( shared_ptr<PredicateOperator> pred : predicates[j] )
            pred->SetForceRender( pred_names[j] ); // want to use later...
    }
    s += "so that we require " + equation->Render() + "\n";
    return s;
}
