#ifndef STAR_AGENT_HPP
#define STAR_AGENT_HPP

#include "agent.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "coupling.hpp"

namespace SR
{ 

// The * wildcard can match more than one node of any type in a container
// In a Sequence, only a contiguous subsequence of 0 or more elements will match
// In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
// Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
// in the collection.
/// Agent used in a container pattern that matches zero or more elements
class StarAgent : public virtual AgentCommon 
{
public:
    virtual bool DecidedCompareImpl( const TreePtrInterface &x,
                                     bool can_key,
                                     Conjecture &conj );
    bool CompareRange( ContainerInterface &range, // range should really be a SubContainer
                       bool can_key );
    virtual TreePtr<Node> BuildReplaceImpl( TreePtr<Node> keynode=TreePtr<Node>() );
private:
    virtual TreePtr<Node> GetPattern() = 0;

public:
    // Internal node classes - NOTE these are not special nodes, and we use them like normal tree nodes
    // Note: only needed for Star pattern
    struct SubContainer : Node // TODO this would be more useful if grid-inherited from ContainerInterface but Node cannot be abstract due NODE_FUNCITONS
    {
        NODE_FUNCTIONS
    };
    struct SubSequenceRange : SequenceInterface,
                              SubContainer
    {
        NODE_FUNCTIONS_FINAL 

        SubSequenceRange() {}
        shared_ptr<iterator_interface> my_begin;
        shared_ptr<iterator_interface> my_end;
    public:
        SubSequenceRange( iterator &b, iterator &e ) : my_begin(b.Clone()), my_end(e.Clone())
        {           
        }
        virtual const iterator_interface &begin() { return *my_begin; }
        virtual const iterator_interface &end()   { return *my_end; }
        virtual void erase( iterator )                      { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual void clear()                                { ASSERTFAIL("Cannot modify SubSequenceRange"); }    
        virtual void insert( const TreePtrInterface & )     { ASSERTFAIL("Cannot modify SubSequenceRange"); }
        virtual TreePtrInterface &operator[]( int i )       { ASSERTFAIL("TODO"); }  
        virtual void push_back( const TreePtrInterface &gx ){ ASSERTFAIL("Cannot modify SubSequenceRange"); }  
    };
    struct SubSequence : Sequence<Node>,
                         SubContainer
    {
        NODE_FUNCTIONS_FINAL 
    };
    struct SubCollection : Collection<Node>,
                           SubContainer
    {
        NODE_FUNCTIONS_FINAL
    };       
};


/// Agent used in a container pattern that matches zero or more elements
template<class PRE_RESTRICTION>
class Star : public StarAgent, 
             public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> pattern; // TODO rename to "restriction"
    virtual TreePtr<Node> GetPattern() 
    {
        return pattern;
    }
};

};

#endif