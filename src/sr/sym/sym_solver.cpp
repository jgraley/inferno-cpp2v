#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "symbol_operators.hpp"
#include "expression_analysis.hpp"
#include "result.hpp"
#include "truth_table_with_predicates.hpp"
#include "conditional_operators.hpp"
#include "set_operators.hpp"
#include "common/lambda_loops.hpp"
#include "rewriters.hpp"

using namespace SYM;

// -------------------------- TruthTableSolver ----------------------------    

TruthTableSolver::TruthTableSolver( const Expression::SolveKit &kit_,
                                    shared_ptr<BooleanExpression> initial_expression_ ) :
    initial_expression( initial_expression_ ),
    kit( kit_ )
{
}


void TruthTableSolver::PreSolve()
{
    const TruthTable::CellType STARTING_VALUE = TruthTable::CellType::TRUE;
 
    // Pre-solve need only be done once for a given initial expression (i.e. once
    // on a given instance of this class) and then any number of solves and/or
    // get alt expression may be performed.
    TRACE("========================================================\nPresolve expression ")
         (initial_expression->Render())("\n");
    
    // Find the predicates and create a truth table of them
    auto predicates = PredicateAnalysis::GetPredicates( initial_expression );
    ttwp = make_unique<TruthTableWithPredicates>( predicates, STARTING_VALUE, label_var_name, counting_based );
    TRACEC(RenderInitialExpressionInTermsOfPredNames())("\n");
    
    // Constrain by searching for derivations of the predicates using rules like
    // substitution or transitivity. Extend the truth table to include derived predicates,
    // constrain according to their rules and fold back down.
    ConstrainUsingDerived();
    TRACEC("Truth Table after derivation: ")(ttwp->Render( {} ))("\n");

    // Constrain (set cells to false) by evaluating our initial expression while the predicates 
    // are all forced to evaluate true or false according to the truth table indices.
    ConstrainByEvaluating();
    TRACEC("Truth Table after boolean evaluation: ")(ttwp->Render( {} ))("\n");
}


shared_ptr<SymbolExpression> TruthTableSolver::TrySolveForGiven( shared_ptr<SymbolVariable> target,
                                                                 const GivenSymbolSet &givens ) const
{
    TRACE("=============================\nSolve for ")
         (target->Render())(" given ")(givens)("\n");
    ASSERT( ttwp )("You need to have done a PreSolve() first\n");
    
    // Sanity: givens are all required by initial expression
    ASSERT( DifferenceOf(givens, initial_expression->GetRequiredVariables()).empty() );
    
    // Sanity: target symbol is required by initial expression
    ASSERT( DifferenceOf(target->GetRequiredVariables(), initial_expression->GetRequiredVariables()).empty() );

    // Sanity: target symbol is not a given
    ASSERT( IntersectionOf(target->GetRequiredVariables(), givens).empty() );

    GivenSymbolSet givens_and_target = UnionOf( givens, target->GetRequiredVariables() );

    // Categorise the preds
    set<shared_ptr<PredicateOperator>> evaluatable_preds;
    map<shared_ptr<PredicateOperator>, shared_ptr<SymbolExpression>> pred_solves;
    for( unsigned axis=0; axis<ttwp->GetDegree(); axis++ )
    {
        auto pred = ttwp->GetFrontPredicate(axis);
        if( !DifferenceOf(pred->GetRequiredVariables(), givens_and_target).empty() )
            continue; // this predicate has required vars that are neither given nor target: exclude 
                      // from analysis (axis will become dead, and will get folded)
        
        if( pred->IsIndependentOf(target) )
            evaluatable_preds.insert( pred );
        else if( shared_ptr<SymbolExpression> solution = pred->TrySolveFor( kit, target ) )
            pred_solves[pred] = solution;
    }
    
    // Get axis numbers for dead axes and fold them out. Dead axes include 
    // - ones that are neither evaulatable nor solvable, i.e. they contain the target but solving failed
    // - as well as ones that refer to symbol variables that were not target or in the "given" set
    set<int> dead_axes;
    for( unsigned axis=0; axis<ttwp->GetDegree(); axis++ )
    {
        auto pred = ttwp->GetFrontPredicate(axis);
        if( evaluatable_preds.count(pred)==0 && pred_solves.count(pred)==0 )
            dead_axes.insert(axis);
    }    
    if( !dead_axes.empty() )
        TRACE("Folding out dead axes:\n")(dead_axes)("\n");
    TruthTableWithPredicates folded_ttwp( ttwp->GetFolded( dead_axes ) );

    // Get axis numbers for the evaluatables and solveables, now that we've finished 
    // folding, which changes them. 
    vector<int> evaluatable_axes, solveable_axes;
    for( unsigned axis=0; axis<folded_ttwp.GetDegree(); axis++ )
        if( pred_solves.count(folded_ttwp.GetFrontPredicate(axis)) ) 
            solveable_axes.push_back(axis);
    TRACE("Truth table after fold out dead: ")(folded_ttwp.Render( ToSet(solveable_axes), false ))("\n");

    // Now make a Karnaugh map around all the non-dead axes (evals and solveables)
    shared_ptr<BooleanExpression> folded_expr = GetExpressionViaKarnaughMap(folded_ttwp);
    TRACE("Expression from folded truth table: ")(folded_expr->Render())("\n");

    // Solve this equation as laid out in #535
    list< shared_ptr<SymbolExpression> > uniands;
    auto terms_in = dynamic_pointer_cast<OrOperator>( folded_expr );
    ASSERT( terms_in );
    for( shared_ptr<BooleanExpression> term_in : terms_in->GetBooleanOperands() )
    {
        list< shared_ptr<SymbolExpression> > interands;
        list< shared_ptr<BooleanExpression> > clauses_out;
        auto clauses_in = dynamic_pointer_cast<AndOperator>( term_in );
        ASSERT( clauses_in );
        for( shared_ptr<BooleanExpression> clause_in : clauses_in->GetBooleanOperands() )
        {            
            auto negated = dynamic_pointer_cast<NotOperator>( clause_in );
            shared_ptr<PredicateOperator> pred;    
            if( negated )
                pred = dynamic_pointer_cast<PredicateOperator>(OnlyElementOf(negated->GetBooleanOperands()));
            else
                pred = dynamic_pointer_cast<PredicateOperator>(clause_in);
            ASSERT( pred );

            if( evaluatable_preds.count(pred) )
            {            
                clauses_out.push_back( clause_in );
            }
            else if( pred_solves.count(pred) )
            {
                shared_ptr<SymbolExpression> interand = pred_solves.at(pred);
            
                // Needed to solve for false. Rather than redo the solve, we can 
                // just complement the solution set.
                if( negated )
                    interands.push_back( make_shared<ComplementOperator>(interand) );
                else
                    interands.push_front( interand ); // non-negated first looks nicer               
            }
        }
        shared_ptr<SymbolExpression> part = make_shared<IntersectionOperator>( interands );
        if( clauses_out.empty() )
        {
            // No restricting clauses, so we always get the intersection
            uniands.push_back( part );
        }
        else
        {
            auto cond = CreateTidiedOperator<AndOperator>(true)( clauses_out );
            auto uniand = make_shared<ConditionalOperator>( cond, part );
            uniands.push_back( uniand );
        }
    }
    auto solution = make_shared<UnionOperator>( uniands );

    TRACE("solution is ")(solution)("\n");

    return solution;
}


shared_ptr<BooleanExpression> TruthTableSolver::GetExpressionViaKarnaughMap( TruthTableWithPredicates initial_ttwp ) const
{
    // Derive a Karnaugh map using TryFindBestKarnaughSlice()
    TruthTableWithPredicates so_far_ttwp = initial_ttwp;
    set<shared_ptr<TruthTable::SliceSpec>> karnaugh_map;
    while( 1 )
    {
        shared_ptr<TruthTable::SliceSpec> slice = 
            initial_ttwp.GetTruthTable().TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far_ttwp.GetTruthTable() );
        if( !slice )
            break; // must have got them all
            
        TRACEC("Got Karnaugh slice: ")(*slice)("\n");
        karnaugh_map.insert( slice );
               
        so_far_ttwp.GetTruthTable().SetSlice(*slice, TruthTable::CellType::FALSE); // Update the TT that indicates progress so far
    }
    
    // Build a union of expressions for the Karnaugh slices
    list< shared_ptr<BooleanExpression> > terms;
    for( shared_ptr<TruthTable::SliceSpec> slice : karnaugh_map )
    {
        ASSERT( slice );
        // Build an intersection of clauses corresponding to solveables
        list< shared_ptr<BooleanExpression> > clauses;
        for( pair<int, bool> p : *slice )
        {
            int axis = p.first;
            int index = p.second;
            auto pred = initial_ttwp.GetFrontPredicate(axis);
            shared_ptr<BooleanExpression> clause = pred;
            
            if( index==false )
                clause = make_shared<NotOperator>(clause);
                
            clauses.push_back( clause );
        }
         
        terms.push_back( make_shared<AndOperator>( clauses ) );
    }

    return make_shared<OrOperator>( terms );
}


shared_ptr<BooleanExpression> TruthTableSolver::GetAltExpressionForTesting() const
{
    // We can create an expression after pre-solve using the truth table 
    // which should evaluate the same as the initial expression, for use as a test.
    
    // We'll make a big consitional. Controls are the predicates.
    vector<shared_ptr<BooleanExpression>> controls;
    for( unsigned ia=0; ia<ttwp->GetDegree(); ia++ )
        controls.push_back( ttwp->GetFrontPredicate(ia) );

    // Options are bool constants taken from the cells of the truth table
    vector<shared_ptr<BooleanExpression>> options;
    ForPower<bool>( ttwp->GetDegree(), index_range_bool, [&](vector<bool> indices)
    {
        TruthTable::CellType cell_value = ttwp->GetTruthTable().Get( indices );      
        bool cell_bool = (cell_value == TruthTable::CellType::TRUE);
        options.push_back( make_shared<BooleanConstant>(cell_bool) );
    } );

    auto solution = make_shared<Multiplexor>( controls, options );

    TRACEC("alternative expression is ")(solution)("\n");

    return solution;
}


void TruthTableSolver::ConstrainByEvaluating()
{
    const TruthTable::CellType SHOULD_EVAL = TruthTable::CellType::TRUE;
    const TruthTable::CellType EVAL_EXCLUDE = TruthTable::CellType::FALSE;
    
    Expression::EvalKit kit { nullptr, nullptr }; 
    
    // Walk the truth table
    ForPower<bool>( ttwp->GetDegree(), index_range_bool, [&](vector<bool> indices)
    {
        ASSERT( indices.size() == ttwp->GetDegree() );
        if( ttwp->GetTruthTable().Get( indices ) == SHOULD_EVAL )
        {            
            // Set up the unique_ptr<BooleanResult>s for the forcing. The forces 
            // apply until these go out of scope. 
            vector<shared_ptr<BooleanExpression>> vbe; // must stay in scope across the Evaluate
            for( bool b : indices )
                vbe.push_back( make_shared<BooleanConstant>(b) );
            
            // Forces must be set up on *all* the predicates that may be reached
            // while evaluating the expression, even if they are equal.
            for( unsigned j=0; j<ttwp->GetDegree(); j++ )
                for( shared_ptr<PredicateOperator> pred : ttwp->GetPredicateSet(j) )
                    pred->SetForceExpression( vbe[j] );       
                
            // Evaluate to find out what the boolean connectives do with forced preds
            unique_ptr<BooleanResult> eval_result = initial_expression->Evaluate(kit);
            
            // Rule out any evaluations that come out false
            if( !eval_result->IsDefinedAndTrue() )
                ttwp->GetTruthTable().Set( indices, EVAL_EXCLUDE );
        }
    } );
}


void TruthTableSolver::ConstrainUsingDerived()
{
    const TruthTable::CellType DERIVE_EXCLUDE = TruthTable::CellType::DONT_CARE;
    
    // Get all the extrapolations into maps, keyed by expression equality
    typedef TruthTableWithPredicates::EqualPredicateSet EqualPredicateSet;

    // These derived_pred_to_... maps will unique-ize on equality of Pk 
    map<shared_ptr<PredicateOperator>, set<set<unsigned>>, Expression::Relation> derived_pred_to_init_indices;
    map<shared_ptr<PredicateOperator>, EqualPredicateSet, Expression::Relation> derived_pred_to_derived_equal_pred_set;
    for( unsigned i=0; i<ttwp->GetDegree(); i++ )
    {
        for( unsigned j=0; j<ttwp->GetDegree(); j++ )
        {
            if( i==j )
                continue;

            auto pi = ttwp->GetFrontPredicate(i);
            auto pj = ttwp->GetFrontPredicate(j);
            
            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = TryDerivePredicate( pi, pj );
            if( pk && !ttwp->PredExists(pk) ) // There is a derivation Pk and it's an extrapolation
            {
                // Record the initial predicates for the extrapolation in an un-ordered way
                derived_pred_to_init_indices[pk].insert( { i, j } );
                
                // Remember all the derivations separately even if equal
                derived_pred_to_derived_equal_pred_set[pk].insert( pk );
            }
        }
    }

    // Get them into vectors, which establishes indices (k) for them. Also filter down to 
    // derivations reached from at least two different sets of initial pred - these are 
    // expected to be the "useful" ones. TODO what about chains of 3, 4 etc?
    vector<EqualPredicateSet> derived_preds;
    for( auto p : derived_pred_to_init_indices )
    {
        TRACEC("Derived predicate ")(p.first->Render())(" appears %d times\n", p.second.size());
        if( p.second.size() >= 2 )
            derived_preds.push_back( derived_pred_to_derived_equal_pred_set.at(p.first) );
    }
    
    // Policy for extending the truth table. Don't make one with degree more than 10
    // also obviously don't bother if there are no extensions.
    int original_degree = ttwp->GetDegree();
    bool should_extend = derived_preds.size() > 0 &&
                         (original_degree+derived_preds.size() <= 10);
    
    // Maybe expand the truth table to include extrapolations
    if( should_extend )    
        ttwp->Extend( derived_preds ); 
    
    // Search all pairs of predicates Pi, Pk for derivations, which should
    // all now be interpolations if we expanded the truth table. If we didn't
    // then we'll now only work with interpolations and ignore extrapolations.
    for( unsigned i=0; i<ttwp->GetDegree(); i++ )
    {
        for( unsigned j=0; j<ttwp->GetDegree(); j++ )
        {
            if( i==j )
                continue;

            // Now we see all distinct pairs (TODO why not use ForAllDistinctPairs()?)
            auto pi = ttwp->GetFrontPredicate(i);
            auto pj = ttwp->GetFrontPredicate(j);
            
            // See whether a relationship can be derived from this pair 
            Relationship rij = TryDeriveRelationship( pi, pj );
            switch( rij )
            {
            case Relationship::NONE:
                break; // nothing to do
            case Relationship::CONTRADICTS:
                TRACEC("Enforcing interpolation: %s ∧ %s => FALSE\n", 
                       PredicateName(i).c_str(), 
                       PredicateName(j).c_str() );  
                ttwp->GetTruthTable().SetSlice( {{i, true}, {j, true}}, DERIVE_EXCLUDE );
                break;
            case Relationship::IMPLIES:
                TRACEC("Enforcing interpolation: %s => %s\n", 
                       PredicateName(i).c_str(), 
                       PredicateName(j).c_str() );  
                ttwp->GetTruthTable().SetSlice( {{i, true}, {j, false}}, DERIVE_EXCLUDE );
                break;
            }

            // See whether a predicate can be derived from this pair 
            shared_ptr<PredicateOperator> pk = TryDerivePredicate( pi, pj );
            if( pk && ttwp->PredExists(pk) ) // There is a derivation Pk and it's an interpolation
            {
                int k = ttwp->PredToIndex(pk);
                // Disallow all combinations that contradict the implication Pi ∧ Pj => Pk
                TRACEC("Enforcing interpolation: %s ∧ %s => %s\n", 
                       PredicateName(i).c_str(), 
                       PredicateName(j).c_str(), 
                       PredicateName(k).c_str() );  
                ttwp->GetTruthTable().SetSlice( {{i, true}, {j, true}, {k, false}}, DERIVE_EXCLUDE );
            }
        }
    }

    // Maybe fold the truth table to simplify out the extrapolations
    if( should_extend )
    {
        set<int> fold_axes;
        for( unsigned k0=0; k0<derived_preds.size(); k0++ )
            fold_axes.insert(original_degree + k0);

        *ttwp = ttwp->GetFolded( fold_axes );
    }
}


Relationship TruthTableSolver::TryDeriveRelationship( shared_ptr<PredicateOperator> pi, 
                                                      shared_ptr<PredicateOperator> pj ) const
{
    ASSERT( Expression::OrderCompare3Way(*pi, *pj) != 0 ); // caller doesn't pass us equal preds
    auto ops_i = pi->GetSymbolOperands();
    auto ops_j = pj->GetSymbolOperands();
    
    if( pi->IsCommutative() || pj->IsCommutative() )
    {
        // Use this un-ordered version if either pred is commutative. Consider > and ==: they
        // contradict regardless of operand order to >.
        
        // Note that taking sets of operands can contract on eg S1 ⚬ S1, but I think
        // the relationship deduction is the same (but most predicates like that are effectively consts)
        auto set_ops_i = ToSet<list<shared_ptr<SymbolExpression>>, Expression::Relation>( ops_i );
        auto set_ops_j = ToSet<list<shared_ptr<SymbolExpression>>, Expression::Relation>( ops_j );
        auto set_ops_common = IntersectionOf( set_ops_i, set_ops_j );
        
        // We want the operands common to both i and j to be either all of i's operands or all of j's
        // This is fine for the common 2-op vs 2-op case, but consider also
        // - some preds like IsInCategoryOperator are 1-op, but can still have relationships with each other (#510)
        // - IsAllDiffOperator is an n-op, but IsEqualOperator (2-op) contradicts any pair of its operands    
        if( set_ops_common.size() < min(set_ops_i.size(), set_ops_j.size()) )
            return Relationship::NONE;
            
        TRACE("Checking for relationship between ")(pi->Render())(" and ")(pj->Render())(" (unordered)\n");
    }
    else 
    {        
        // Use this order-respecting version if both preds are non-commutative.
        // We need identical lists of identical operands
        // TODO: we should be able to do something if Pj's operands are 
        // reversed. Maybe need a GetCommute() on non-commutative preds?
        // So if the lex compare with reversed j ops matches we do
        // pi->GetRelationshipWith( pj->GetCommute() ). Could be used for 
        // REVERSE transitivity too.
        if( LexicographicalCompare( ops_i, ops_j, Expression::Relation() ) != 0 )
            return Relationship::NONE;
            
        if( ops_i.size()==1 && ops_j.size()==1 )
        {
            // Supposed to be the same operand
            
            // Check using OrderCompareEqual()
            ASSERT( Expression::OrderCompare3Way( *OnlyElementOf(ops_i), *OnlyElementOf(ops_j) ) == 0 );
            
            // Check by looking directly
            if( auto svi = dynamic_pointer_cast<SymbolVariable>(OnlyElementOf(ops_i)) )
                if( auto svj = dynamic_pointer_cast<SymbolVariable>(OnlyElementOf(ops_j)) )
                    ASSERT( svi->GetPatternLink() == svj->GetPatternLink() );
        }

        TRACE("Checking for relationship between ")(pi->Render())(" and ")(pj->Render())(" (ordered)\n");
    }
            
    return pi->GetRelationshipWith( pj );    
}
                                                      


shared_ptr<PredicateOperator> TruthTableSolver::TryDerivePredicate( shared_ptr<PredicateOperator> pi, 
                                                                    shared_ptr<PredicateOperator> pj ) const
{
    ASSERT( Expression::OrderCompare3Way(*pi, *pj) != 0 ); // caller doesn't pass us equal preds
    
    // Try to substitute one variable with another 
    if( pi->IsCanSubstituteFrom() ) // basically EqualsOperator
    {
        auto ops_i = ToPair( pi->GetSymbolOperands() );

        // Try both ways round. 
        if( shared_ptr<PredicateOperator> pk = pj->TrySubstitute( ops_i.first, ops_i.second ) )
            return pk;

        if( shared_ptr<PredicateOperator> pk = pj->TrySubstitute( ops_i.second, ops_i.first ) )
            return pk;        
            
        return nullptr;
    }
    
    // Derive the implications of transitive operators
    Transitivity tij = pi->GetTransitivityWith( pj );
    if( tij != Transitivity::NONE )
    {
        // Operators must be sized right if transitivity is announced
        auto ops_i = ToPair( pi->GetSymbolOperands() );
        auto ops_j = ToPair( pj->GetSymbolOperands() );

        // Try forward case 
        if( (tij==Transitivity::FORWARD || tij==Transitivity::BIDIRECTIONAL) )
        {
            if( Expression::OrderCompare3Way(*ops_i.second, *ops_j.first) == 0 )
                return Substitute( pi, ops_i.second, ops_j.second ); // Textbook case           

            if( Expression::OrderCompare3Way(*ops_i.first, *ops_j.second) == 0 )
                return Substitute( pi, ops_i.first, ops_j.first );                
        }
        
        // Try reverse case, where the second pred (pj) is commuted
        if( (tij==Transitivity::REVERSE || tij==Transitivity::BIDIRECTIONAL) )
        {
            if( Expression::OrderCompare3Way(*ops_i.second, *ops_j.second) == 0 )
                return Substitute( pi, ops_i.second, ops_j.first );                

            if( Expression::OrderCompare3Way(*ops_i.first, *ops_j.first) == 0 )
                return Substitute( pi, ops_i.first, ops_j.second );    
        }
    }
    
    return nullptr;
}
                                                               

shared_ptr<PredicateOperator> TruthTableSolver::Substitute( shared_ptr<PredicateOperator> pred,
                                                            shared_ptr<SymbolExpression> over,
                                                            shared_ptr<SymbolExpression> with ) const
{
    shared_ptr<PredicateOperator> new_pred = pred->TrySubstitute( over, with );    
    ASSERT( new_pred ); // must succeed
    return new_pred;    
}                                                            


string TruthTableSolver::PredicateName(int j)
{
    return label_var_name + to_string(j+counting_based);
}


string TruthTableSolver::RenderInitialExpressionInTermsOfPredNames()
{
    string s;
    vector<shared_ptr<string>> pred_names(ttwp->GetDegree());
    for( unsigned j=0; j<ttwp->GetDegree(); j++ )
    {
        pred_names[j] = make_shared<string>(PredicateName(j));
        for( shared_ptr<PredicateOperator> pred : ttwp->GetPredicateSet(j) )
            pred->SetForceRender( pred_names[j] ); // want to use later...
    }
    s += "We require " + initial_expression->Render() + "\n";
    return s;
}
