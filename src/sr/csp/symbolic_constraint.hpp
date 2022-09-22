#ifndef SYMBOLIC_CONSTRAINT_HPP
#define SYMBOLIC_CONSTRAINT_HPP

#include "constraint.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../sym/expression.hpp"
#include "../sym/rewriters.hpp"

#include <memory>
#include <list>

namespace SR
{
class Conjecture;    
class XTreeDatabase;
}

namespace CSP
{ 

/** Implements a systemic constraint as discussed in #107
 */
class SymbolicConstraint : public Constraint
{
public:    
    /**
     * Create the constraint. 
     * 
     * The constraint is permanently associated with the supplied symbolic
     * boolean operator.
     * 
     * @param op a shared pointer to the boolean operator
     */
    explicit SymbolicConstraint( shared_ptr<SYM::BooleanExpression> op,
                                 shared_ptr<const SR::XTreeDatabase> x_tree_db );
    
private:
    const struct Plan : public virtual Traceable
    {
        explicit Plan( SymbolicConstraint *algo,  
                       shared_ptr<SYM::BooleanExpression> expression,
                       shared_ptr<const SR::XTreeDatabase> x_tree_db );
        void DetermineVariables();
        void DetermineHintExpressions();
        void DetermineXTreeDbRequirement();
        string GetTrace() const; // used for debug

        SymbolicConstraint * const algo;
        shared_ptr<SYM::BooleanExpression> consistency_expression;        
        set<VariableId> variables;
        typedef map< set<VariableId>, shared_ptr<SYM::SymbolExpression>> GivensToExpression;
        map<VariableId, GivensToExpression> suggestion_expressions;
        shared_ptr<SYM::BooleanExpression> alt_expression_for_testing;       
        SYM::Expression::VariablesRequiringRows required_x_tree_db_level; 
        shared_ptr<const SR::XTreeDatabase> x_tree_db;
    } plan;

    const set<VariableId> &GetVariables() const override;
    SYM::Expression::VariablesRequiringRows GetVariablesRequiringRows() const override;
    virtual void Start();    
    bool IsSatisfied( const Assignments &assignments ) const override;
    unique_ptr<SYM::SetResult> GetSuggestedValues( const Assignments &assignments,
                                                   const VariableId &var ) const override;               
    string GetTrace() const override;
    void Dump() const;
};

};

#endif
