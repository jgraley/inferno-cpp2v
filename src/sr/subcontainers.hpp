#ifndef SUBCONTAINERS
#define SUBCONTAINERS

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{
    
// Internal node classes - NOTE these are not agents, they are local tree nodes

struct SubContainer : Node 
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
#if SEQUENCE_HAS_RANDOM_ACCESS
    virtual TreePtrInterface &operator[]( int i )       { ASSERTFAIL("TODO"); }  
#endif
    virtual void push_back( const TreePtrInterface &gx ){ ASSERTFAIL("Cannot modify SubSequenceRange"); }  
};


struct SubSequence : Sequence<Node>,
                     SubContainer
{
    NODE_FUNCTIONS_FINAL 
    SubSequence &operator=( Sequence<Node> o )
    {
        (void)Sequence<Node>::operator=(o);
        return *this;
    }
};


struct SubCollection : Collection<Node>,
                       SubContainer
{
    NODE_FUNCTIONS_FINAL
    SubCollection &operator=( Collection<Node> o )
    {
        (void)Collection<Node>::operator=(o);
        return *this;
    }
};       

};
#endif