#ifndef GENERICS_HPP
#define GENERICS_HPP

#include "common/refcount.hpp"
#include <deque>
#include "itemise_members.hpp"

struct Node;

struct GenericSharedPtr : Itemiser::Element
{
    virtual shared_ptr<Node> Get() = 0;
    virtual void Set( shared_ptr<Node> n ) = 0;
};

struct GenericSequence : Itemiser::Element
{
    virtual GenericSharedPtr &Element( int i ) = 0;
    virtual int size() const = 0;
};

template<typename ELEMENT>
struct SharedPtr : GenericSharedPtr, shared_ptr<ELEMENT> 
{
    virtual shared_ptr<Node> Get()
    {
        shared_ptr<ELEMENT> *p = (shared_ptr<ELEMENT> *)this;
        ASSERT(p);
        shared_ptr<Node> n = (shared_ptr<Node>)*p;
        return n;
    }
    virtual void Set( shared_ptr<Node> n )
    {
        if( !n )
        {
            *(shared_ptr<ELEMENT> *)this = shared_ptr<ELEMENT>(); // handle NULL explicitly since dyn cast uses NULL to indicate wrong type   
        }
        else
        {
            shared_ptr<ELEMENT> pe = dynamic_pointer_cast<ELEMENT>(n);
            if( !pe )
                TRACE("Type was %s; I am %s\n", TypeInfo(n).name().c_str(), typeid(ELEMENT).name() );
            ASSERT( pe && "Tried to Set() wrong type of node via GenericSharedPtr" );
            *(shared_ptr<ELEMENT> *)this = pe;        
        }
    }
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>(o) {}
    SharedPtr() {}
};           

template<typename ELEMENT>
struct Sequence : GenericSequence, deque< SharedPtr<ELEMENT> > 
{
    virtual GenericSharedPtr &Element( int i )
    {
        return deque< SharedPtr<ELEMENT> >::operator[](i);
    }
    virtual int size() const
    {
        return deque< SharedPtr<ELEMENT> >::size();
    }
};

#endif
