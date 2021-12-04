#ifndef SYMBOLIC_CONSTRAINT_HPP
#define SYMBOLIC_CONSTRAINT_HPP

#include "constraint.hpp"

#include "node/specialise_oostd.hpp"
#include "common/common.hpp"
#include "../symbolic/expression.hpp"

#include <memory>
#include <list>

namespace SR
{
class Conjecture;    
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
                                 set<SR::PatternLink> relevent_plinks );
    
private:
    const struct Plan : public virtual Traceable
    {
        explicit Plan( SymbolicConstraint *algo,  
                       shared_ptr<SYM::BooleanExpression> op,
                       set<SR::PatternLink> relevent_plinks );
        void DetermineVariables( set<SR::PatternLink> relevent_plinks );
        string GetTrace() const; // used for debug

        SymbolicConstraint * const algo;
        shared_ptr<SYM::BooleanExpression> op;
        list<VariableId> variables;
    } plan;

    const list<VariableId> &GetVariables() const override;
    virtual void Start( const Assignments &forces_map_, 
                        const SR::TheKnowledge *knowledge_ );    
    tuple<bool, Assignment> Test( Assignments frees_map ) override;
            
    list<Value> forces;
    Assignments forces_map;
    const SR::TheKnowledge *knowledge;
    
    void Dump() const;
};

};

#endif
