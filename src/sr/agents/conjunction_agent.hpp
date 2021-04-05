#ifndef CONJUNCTION_AGENT_HPP
#define CONJUNCTION_AGENT_HPP

#include "../search_replace.hpp"
#include "colocated_agent.hpp"
#include "../boolean_evaluator.hpp"

namespace SR
{

/// Boolean node that matches if all of the sub-patterns at the pointers in
/// `patterns` do match i.e. an "and" operation. `patterns` point to  
/// normal contexts, since the global and-rule is preserved.
class ConjunctionAgent : public virtual ColocatedAgent 
{
public:               
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
private:
    virtual CollectionInterface &GetPatterns() const = 0;
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
    
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual CollectionInterface &GetPatterns() const
    {
        return patterns;
    }
};

};

#endif
