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

    typedef set<int> InitialPreds;
    typedef shared_ptr<PredicateOperator> DerivedPred;
    map<DerivedPred, set<InitialPreds>, Expression::OrderComparer> predmap;
    for( int i=0; i<predicates.size(); i++ )
    {
        for( int j=0; j<predicates.size(); j++ )
        {
            if( i==j )
                continue;

            auto pi = FrontOf(predicates[i]);
            auto pj = FrontOf(predicates[j]);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pop = pi->TryDerive( pj );
            if( pop && pred_to_index.count(pop)==0 )
            {
                set<InitialPreds> &sip = predmap[pop];
                sip.insert( { i, j } );
            }
        }
    }

    vector<pair<InitialPreds, DerivedPred>> derived_predicates;
    for( auto p : predmap )
    {
        TRACE("Derived predicate ")(p.first->Render())(" appears %d times\n", p.second.size());
        if( p.second.size() > 1 )
        {
            pair<InitialPreds, DerivedPred> pp = make_pair(FrontOf(p.second), p.first);
            derived_predicates.push_back( pp );
        }
    }
    
    bool should_expand = (predicates.size()+derived_predicates.size() <= 10);
    if( should_expand )
    {
        TRACE("Extending truth table from %d by %d\n", predicates.size(), derived_predicates.size());
        truth_table->Extend( predicates.size() + derived_predicates.size() ); 
    }
    
    pred_to_index.clear();
    for( int i=0; i<predicates.size(); i++ )
        pred_to_index[FrontOf(predicates[i])] = i;
    for( int i=0; i<predicates.size(); i++ )
    {
        for( int j=0; j<predicates.size(); j++ )
        {
            if( i==j )
                continue;

            auto pi = FrontOf(predicates[i]);
            auto pj = FrontOf(predicates[j]);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pop = pi->TryDerive( pj );
            if( pop )
            {
#if 0 // ASSERT failing
                if( should_expand )
                    ASSERT( pred_to_index.count(pop)==1 )("Should have expanded the truth table to include all extrapolations\npop=")( pop )("\npred_to_index=\n")( pred_to_index );
                int k = pred_to_index.at(pop);
                truth_table->SetSlice( {{i, true}, {j, true}, {k, false}}, false );
#endif                
            }
        }
    }

    if( should_expand )
    {
        set<int> fold_axes;
        for( int k0=0; k0<derived_predicates.size(); k0++ )
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
