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
class TheKnowledge;
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
                                 shared_ptr<const SR::TheKnowledge> knowledge );
    
private:
    const struct Plan : public virtual Traceable
    {
        explicit Plan( SymbolicConstraint *algo,  
                       shared_ptr<SYM::BooleanExpression> expression,
                       shared_ptr<const SR::TheKnowledge> knowledge );
        void DetermineVariables();
        void DetermineHintExpressions();
        void DetermineKnowledgeRequirement();
        string GetTrace() const; // used for debug

        SymbolicConstraint * const algo;
        shared_ptr<SYM::BooleanExpression> consistency_expression;        
        set<VariableId> variables;
        typedef map< set<VariableId>, shared_ptr<SYM::SymbolExpression>> GivensToExpression;
        map<VariableId, GivensToExpression> suggestion_expressions;
        shared_ptr<SYM::BooleanExpression> alt_expression_for_testing;       
        SYM::Expression::VariablesRequiringNuggets required_knowledge_level; 
        shared_ptr<const SR::TheKnowledge> knowledge;
    } plan;

    const set<VariableId> &GetVariables() const override;
    SYM::Expression::VariablesRequiringNuggets GetVariablesRequiringNuggets() const override;
    virtual void Start( const SR::TheKnowledge *knowledge_ );    
    bool IsConsistent( const Assignments &assignments ) const override;
    unique_ptr<SYM::SetResult> GetSuggestedValues( const Assignments &assignments,
                                                   const VariableId &var ) const override;               
    
    const SR::TheKnowledge *knowledge;
    
    void Dump() const;
};

};

#endif
