#ifndef CLUTCH_HPP
#define CLUTCH_HPP

#include "expression.hpp"

namespace SYM
{         

// ------------------------- ClutchRewriter --------------------------

// The clutch idea works like this:
//  - CSP solvers will only find solutions when all constraints are 
//    satisfied. Constraints are made from symbolic expressions.
//  - There are symbolic expressions that we wish to make "optional"
//  - This is likened to disengaging a clutch in a car.
//  - Disengagement occurs when the required variables match the 
//    disengager expression, usually a constant (MMAX)
//  - Given disengager and original expressions, the clutched
//    version is just a combination of these and some new logic.
class ClutchRewriter
{
public:    
    explicit ClutchRewriter( shared_ptr<SymbolExpression> disengager );
    
    // Wrap the given expression in clutch logic, using GetRequiredVariables() to
    // determine which variables should be de-clutched by the disengager expression
    shared_ptr<BooleanExpression> ApplyUnified(shared_ptr<BooleanExpression> original_expr) const;

    // Apply the unified rewrite individually to each clause of a conjunctive expression
    shared_ptr<BooleanExpression> ApplyDistributed(shared_ptr<BooleanExpression> original_expr) const;
    
private:
    const shared_ptr<SymbolExpression> disengager_expr;
};

};

#endif
