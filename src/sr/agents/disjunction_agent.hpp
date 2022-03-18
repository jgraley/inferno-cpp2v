#ifndef DISJUNCTION_AGENT_HPP
#define DISJUNCTION_AGENT_HPP

#include "agent_common.hpp"
#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"
#include "standard_agent.hpp"
#include "../sym/predicate_operators.hpp"

namespace SR
{

/// Boolean node that matches if any of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "or" operation. `patterns` point to abnormal 
/// contexts since in an overall match, some sub-patterns may not match.
class DisjunctionAgent : public virtual AgentCommon
{
public:
    class NoOptionsMatchedMismatch : public Mismatch {};
    class TakenOptionMismatch : public Mismatch {};
    class MMAXRequiredOnUntakenOptionMismatch : public Mismatch {};
    
    // Only to prevent crash when hypothesis_links is empty eg during truth table pre-solve
    class IncompleteQuery : public Mismatch {}; 

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;                                
                                                               
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryImpl() const;                                       

    virtual Block GetGraphBlockInfo() const;
    
    // Interface for pattern trasformation
    virtual void SetDisjuncts( CollectionInterface &ci ) = 0;
    virtual CollectionInterface &GetDisjuncts() const = 0;
    virtual TreePtr<Node> CloneToEmpty() const = 0;
    
private:
    virtual void SCRConfigure( const SCREngine *e,
                               Phase phase );
    shared_ptr< Collection<Node> > options;

    class WideMainOperator : public SYM::PredicateOperator
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit WideMainOperator( shared_ptr<SYM::SymbolExpression> keyer,
                                  list<shared_ptr<SYM::SymbolExpression>> disjuncts  ); 
        virtual list<shared_ptr<SYM::SymbolExpression>> GetSymbolOperands() const override;
        virtual shared_ptr<SYM::BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                         const list<shared_ptr<SYM::SymbolResultInterface>> &op_results ) const override;

        virtual string RenderNF() const override;
        virtual Precedence GetPrecedenceNF() const override;
        
    protected:
        shared_ptr<SYM::SymbolExpression> keyer;
        list<shared_ptr<SYM::SymbolExpression>> disjuncts;
    };
    
    class WideMainBoolOperator : public SYM::BooleanToBooleanExpression
    {
    public:    
        typedef BooleanExpression NominalType;
        explicit WideMainBoolOperator( list<shared_ptr<SYM::BooleanExpression>> is_keyer_disjuncts,
                                       list<shared_ptr<SYM::BooleanExpression>> is_mmax_disjuncts ); 
        virtual list<shared_ptr<SYM::BooleanExpression>> GetBooleanOperands() const override;
        virtual shared_ptr<SYM::BooleanResultInterface> Evaluate( const EvalKit &kit,
                                                                  const list<shared_ptr<SYM::BooleanResultInterface>> &op_results ) const override;

        virtual string Render() const override;
        virtual Precedence GetPrecedence() const override;
        
    protected:
        int num_disjuncts;
        list<shared_ptr<SYM::BooleanExpression>> is_keyer_disjuncts;
        list<shared_ptr<SYM::BooleanExpression>> is_mmax_disjuncts;
    };
};


template<class PRE_RESTRICTION>
class Disjunction : public Special<PRE_RESTRICTION>,
                    public DisjunctionAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    // Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> disjuncts; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetDisjuncts() const override
    {
        return disjuncts;
    }
    
    virtual void SetDisjuncts( CollectionInterface &ci ) override
    {
        // Note: options should not have been set yet during ptrans so 
        // only need to update patterns
        disjuncts.clear();
        for( CollectionInterface::iterator pit = ci.begin(); 
             pit != ci.end(); 
             ++pit )    
            disjuncts.insert( TreePtr<PRE_RESTRICTION>::DynamicCast(*pit) );      
    }
    
    virtual TreePtr<Node> CloneToEmpty() const override
    {
        return MakePatternPtr<Disjunction<PRE_RESTRICTION>>();
    }
    
};

};

#endif
