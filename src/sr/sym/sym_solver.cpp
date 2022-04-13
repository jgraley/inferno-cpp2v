#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "expression_analysis.hpp"
#include "result.hpp"
#include "truth_table_with_predicates.hpp"
#include "conditional_operators.hpp"
#include "set_operators.hpp"

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


void SymSolver::PreSolve()
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolveFor( shared_ptr<SymbolExpression> target ) const
{
    shared_ptr<Expression> solution = equation->TrySolveForToEqual( target,
                                                                    make_shared<BooleanConstant>(true) );
    if( !solution )
        return nullptr;
    
    auto sym_solution = dynamic_pointer_cast<SymbolExpression>(solution);
    ASSERT( sym_solution )(solution->Render()); // target is shared_ptr<SymbolicExpression> so should get symbol as solution
    return sym_solution;
}


shared_ptr<BooleanExpression> SymSolver::GetAltEquationForTesting() const
{
    return nullptr; // feature not supported
}


// -------------------------- TruthTableSolver ----------------------------    

TruthTableSolver::TruthTableSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


void TruthTableSolver::PreSolve()
{
    TRACE("Presolve equation: ")(equation->Render())("\n");
    
    auto predicates = PredicateAnalysis::GetPredicates( equation );
    ttwp = make_unique<TruthTableWithPredicates>( predicates, true, label_var_name, counting_based );

    TRACEC(RenderEquationInTermsOfPreds())("\n");
    
    PopulateInitial();

    TRACEC(ttwp->Render( {} ))("\n");

    ConstrainUsingDerived();
    
    TRACEC(ttwp->Render( {} ))("\n");
}


shared_ptr<SymbolExpression> TruthTableSolver::TrySolveFor( shared_ptr<SymbolExpression> target,
                                                            const GivenSymbolSet &givens ) const
{
    TRACE("Solve equation: ")(equation->Render())(" for ")(target->Render())("\n");
    ASSERT( ttwp )("You need to have done a PreSolve() first\n");
    
    // Sanity: givens are all required by equation
    ASSERT( DifferenceOf(givens, equation->GetRequiredVariables()).empty() );
    
    // Sanity: target symbol is required by equation
    ASSERT( DifferenceOf(target->GetRequiredVariables(), equation->GetRequiredVariables()).empty() );

    // Sanity: target symbol is not a given
    ASSERT( IntersectionOf(target->GetRequiredVariables(), givens).empty() );

    GivenSymbolSet givens_and_target = UnionOf( givens, target->GetRequiredVariables() );

    // Categorise the preds
    set<shared_ptr<PredicateOperator>> evaluatable_preds, solveable_preds;
    for( int axis=0; axis<ttwp->GetDegree(); axis++ )
    {
        auto pred = ttwp->GetFrontPredicate(axis);
        if( !DifferenceOf(pred->GetRequiredVariables(), givens_and_target).empty() )
            continue; // this predicate has required vars that are not given/target: exclude 
                      // from analysis (axis will become dead, and will get folded)
        
        if( pred->IsIndependentOf(target) )
            evaluatable_preds.insert( pred );
        else if( pred->TrySolveForToEqual( target, make_shared<BooleanConstant>(true) ) )
            solveable_preds.insert( pred );
    }
    
    // Get axis numbers for dead axes and fold them out. Dead axes include 
    // - ones that are neither evaulatable nor solvable, i.e. they contain the target but solving failed
    // - as well as ones that refer to symbol variables that were not in the "given" set
    set<int> dead_axes;
    for( int axis=0; axis<ttwp->GetDegree(); axis++ )
    {
        auto pred = ttwp->GetFrontPredicate(axis);
        if( evaluatable_preds.count(pred)==0 && solveable_preds.count(pred)==0 )
            dead_axes.insert(axis);
    }    
    if( !dead_axes.empty() )
        TRACEC("Folding out dead axes:\n")(dead_axes)("\n");
    TruthTableWithPredicates folded_ttwp( ttwp->GetFolded( dead_axes, false ) );
    
    // Get axis numbers for the evaluatable 
    vector<int> evaluatable_axes, solveable_axes;
    for( int axis=0; axis<folded_ttwp.GetDegree(); axis++ )
    {
        auto pred = folded_ttwp.GetFrontPredicate(axis);
        if( evaluatable_preds.count(pred) )
            evaluatable_axes.push_back(axis);
        if( solveable_preds.count(pred) ) 
            solveable_axes.push_back(axis);
    }
    TRACEC(folded_ttwp.Render( ToSet(solveable_axes) ))("\n");

    // Controls in the multiplexor will be the evauators
    vector<shared_ptr<BooleanExpression>> controls;
    for( int axis : evaluatable_axes )
    {
        auto pred = folded_ttwp.GetFrontPredicate(axis);
        controls.push_back( pred );
    }

    // Solve the soveables into a map opf solutions
    map<shared_ptr<PredicateOperator>, shared_ptr<SymbolExpression>> solution_map;
    for( int axis : solveable_axes )
    {
        auto pred = folded_ttwp.GetFrontPredicate(axis);
        shared_ptr<Expression> esolution = pred->TrySolveForToEqual( target, make_shared<BooleanConstant>(true) );
        if( !esolution )   // NULL means failed to solve
            continue;
        
        auto solution = dynamic_pointer_cast<SymbolExpression>( esolution );
        ASSERT( solution );  
        solution_map[pred] = solution;
    }

    // Make up the clauses using set operations on the solutions 
    vector<shared_ptr<SymbolExpression>> options;
    ForPower( evaluatable_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> evaluatable_indices)
    {
        map<int, bool> evaluatable_indices_map;
        for( int i=0; i<evaluatable_axes.size(); i++ )
            evaluatable_indices_map[evaluatable_axes.at(i)] = evaluatable_indices.at(i);
        TruthTableWithPredicates evaluated_ttwp( folded_ttwp.GetSlice( evaluatable_indices_map ) ); 
        TRACEC("evaluated_ttwp ")(evaluated_ttwp.Render({}))("\n");
        
        set<vector<bool>> permitted_terms = evaluated_ttwp.GetTruthTable().GetIndicesOfValue( true );
        TRACEC("Permitted terms ")(permitted_terms)("\n");

        // Build a union of terms that were not ruled out
        list< shared_ptr<SymbolExpression> > terms;
        for( vector<bool> term : permitted_terms )
        {
            // Build an intersection of clauses corresponding to solveables
            list< shared_ptr<SymbolExpression> > clauses;
            for( int axis=0; axis<term.size(); axis++ )
            {
                auto pred = evaluated_ttwp.GetFrontPredicate(axis);
                if( solution_map.count(pred) == 0 )
                    continue; // skip where solve failed
                shared_ptr<SymbolExpression> clause = solution_map.at(pred);
                
                // Needed to solve for false. Rather than redo the solve, we can 
                // just complement the solution set.
                if( term.at(axis)==false )
                    clause = make_shared<ComplementOperator>(clause);
                    
                clauses.push_back( clause );
            }
             
            terms.push_back( make_shared<IntersectionOperator>( clauses ) );
        }

        options.push_back( make_shared<UnionOperator>( terms ) );
    } );

    auto solution = make_shared<MultiConditionalOperator>( controls, options );

    TRACEC("solution is ")(solution)("\n");

    return solution;
}


shared_ptr<BooleanExpression> TruthTableSolver::GetAltEquationForTesting() const
{
    vector<shared_ptr<BooleanExpression>> controls;
    for( int ia=0; ia<ttwp->GetDegree(); ia++ )
    {
        controls.push_back( ttwp->GetFrontPredicate(ia) );
    }

    vector<shared_ptr<BooleanExpression>> options;
    ForPower( ttwp->GetDegree(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> indices)
    {
        bool cell_value = ttwp->GetTruthTable().Get( indices );      
        options.push_back( make_shared<BooleanConstant>(cell_value) );
    } );

    auto solution = make_shared<MultiBooleanConditionalOperator>( controls, options );

    TRACEC("alternative expression is ")(solution)("\n");

    return solution;
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
    vector<set<DerivedPred>> derived_preds;
    for( auto p : derived_pred_to_init_indices )
    {
        TRACEC("Derived predicate ")(p.first->Render())(" appears %d times\n", p.second.size());
        if( p.second.size() > 1 )
            derived_preds.push_back( derived_pred_to_equal_derived_preds.at(p.first) );
    }
    
    // Policy for extending the truth table. Don't make one with degree more than 10
    // also obviously don't bother if there are no extensions.
    int original_degree = ttwp->GetDegree();
    bool should_extend = derived_preds.size() > 0 &&
                         (original_degree+derived_preds.size() <= 10);
    
    if( should_extend )    
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
                    TRACEC("Enforcing interpolation: %s ∧ %s => %s\n", PredicateName(i).c_str(), PredicateName(j).c_str(), PredicateName(k).c_str() );  
                    ttwp->GetTruthTable().SetSlice( {{i, true}, {j, true}, {k, false}}, false );
                }
            }
        }
    }

    if( should_extend )
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
