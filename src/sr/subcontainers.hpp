#ifndef SUBCONTAINERS
#define SUBCONTAINERS

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"

namespace SR
{
    
// Internal node classes - NOTE these are not agents, they are local tree nodes

struct SubContainer : Node // TODO #69
{
    NODE_FUNCTIONS
};


struct SubContainerRange : SubContainer
{    
    NODE_FUNCTIONS
    
    SubContainerRange( TreePtr<Node> parent_x_ = nullptr ) :
        parent_x( parent_x_ )
    {
    }
    
    TreePtr<Node> GetParentX()
    {
        return parent_x;
    }
    
    TreePtr<Node> parent_x;
};


struct SubSequenceRange : SequenceInterface,
                          SubContainerRange
{
    NODE_FUNCTIONS_FINAL 

    SubSequenceRange() {}
    shared_ptr<iterator_interface> my_begin;
    shared_ptr<iterator_interface> my_end;
public:
    SubSequenceRange( TreePtr<Node> parent_x, iterator &b, iterator &e );
    virtual const iterator_interface &begin();
    virtual const iterator_interface &end();
    virtual void erase( const iterator_interface & )    { ASSERTFAIL("Cannot modify SubSequenceRange"); }
    virtual void clear()                                { ASSERTFAIL("Cannot modify SubSequenceRange"); }    
    virtual void insert( const TreePtrInterface & )     { ASSERTFAIL("Cannot modify SubSequenceRange"); }
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