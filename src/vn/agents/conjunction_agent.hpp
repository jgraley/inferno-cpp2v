#ifndef CONJUNCTION_AGENT_HPP
#define CONJUNCTION_AGENT_HPP

#include "../search_replace.hpp"
#include "autolocating_agent.hpp"
#include "../boolean_evaluator.hpp"

namespace VN
{

/// Boolean node that matches if all of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "and" operation. `patterns` point to  
/// normal contexts, since the global and-rule is preserved.
class ConjunctionAgent : public virtual AutolocatingAgent 
{
public:               
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;

    ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                          PatternLink me_plink, 
                                          XLink key_xlink,
                                          const SCREngine *acting_engine ) final;

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    NodeBlock GetGraphBlockInfo() const final;

private:
    virtual CollectionInterface &GetConjuncts() const = 0;
};


template<class PRE_RESTRICTION>
class Conjunction : public Special<PRE_RESTRICTION>,
                    public ConjunctionAgent
{
public:
    SPECIAL_NODE_FUNCTIONS
    
    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    mutable Collection<PRE_RESTRICTION> conjuncts; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetConjuncts() const
    {
        return conjuncts;
    }
    string GetCouplingNameHint() const final
    {
		return "all"; 
	}};

};

#endif
