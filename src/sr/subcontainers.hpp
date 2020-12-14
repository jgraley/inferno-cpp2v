#ifndef SUBCONTAINERS
#define SUBCONTAINERS

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "link.hpp"    

#include <list>
#include <set>

namespace SR
{
    
// Internal node classes - NOTE these are not agents, they are local tree nodes

struct SubContainer : Node // TODO #69
{
    NODE_FUNCTIONS
    
    // XLinks to these are often created using CreateDistinct()
    // (putting them in the domain would make the domain too big).
    // These functions operate on links held within the subcontainer,
    // which typically will be in the domain.
    virtual string GetContentsTrace() { return ""; } 
    virtual void AssertMatchingContents( TreePtr<Node> other ) { ASSERTFAIL(); }
};


// This kind of subcontainer carries a range (begin/end pair) on the 
// parent X node's container, as well as a pointer to the parent. This
// is sufficient to enable the correct XLinks to be built when needed.
struct SubContainerRange : SubContainer,
                           virtual ContainerInterface // virtual required to allow subclasses to use my impl for interfaces they bring in
                           
{    
    NODE_FUNCTIONS
    
    SubContainerRange() {}
    TreePtr<Node> parent_x;
    shared_ptr<iterator_interface> my_begin;
    shared_ptr<iterator_interface> my_end;
public:
    SubContainerRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e );
    virtual const iterator_interface &begin();
    virtual const iterator_interface &end();
    virtual void erase( const iterator_interface & )    { ASSERTFAIL("Cannot modify SubContainerRange"); }
    virtual void clear()                                { ASSERTFAIL("Cannot modify SubContainerRange"); }    
    virtual void insert( const TreePtrInterface & )     { ASSERTFAIL("Cannot modify SubContainerRange"); }
    virtual void push_back( const TreePtrInterface &gx ){ ASSERTFAIL("Cannot modify SubContainerRange"); }  
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    
    
    TreePtr<Node> GetParentX()
    {
        return parent_x;
    }    
};


struct SubSequenceRange : SubContainerRange,
                          SequenceInterface
{
    SubSequenceRange() {}
    SubSequenceRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e ) :
        SubContainerRange( parent_x, b, e )
    {
    }
    
    NODE_FUNCTIONS_FINAL 
};


// This kind of subcontainer stores all the XLinks for retrieval later
// TODO no need for Sequence<Node> any more I believe
struct SubContainerLinks : SubContainer
{    
    NODE_FUNCTIONS
};


struct SubSequence : Sequence<Node>,
                     SubContainerLinks
{
    NODE_FUNCTIONS_FINAL 
    SubSequence &operator=( Sequence<Node> o )
    {
        (void)Sequence<Node>::operator=(o);
        return *this;
    }
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    

    list<XLink> elts;
};


struct SubCollection : Collection<Node>, 
                       SubContainerLinks
{
    NODE_FUNCTIONS_FINAL
    SubCollection &operator=( Collection<Node> o )
    {
        (void)Collection<Node>::operator=(o);
        return *this;
    }
    virtual string GetContentsTrace(); 
    virtual void AssertMatchingContents( TreePtr<Node> other );    
    
    set<XLink> elts;
};       

};
#endif