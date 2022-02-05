#ifndef DISJUNCTION_AGENT_HPP
#define DISJUNCTION_AGENT_HPP

#include "agent_common.hpp"
#include "../search_replace.hpp"
#include "../boolean_evaluator.hpp"
#include "standard_agent.hpp"

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

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink x ) const;                                    
                                                               
    virtual bool ImplHasSNLQ() const;
    virtual void RunNormalLinkedQueryImpl( const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;                                                                                        
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
