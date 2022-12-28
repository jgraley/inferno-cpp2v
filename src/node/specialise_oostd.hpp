#ifndef SPECIALISE_OOSTD_HPP
#define SPECIALISE_OOSTD_HPP

#include "common/common.hpp"
#include "tree_ptr.hpp"
#include "containers.hpp"
#include <list>
#include <set>
#include <iterator>

/** Produce a container around an iterator that has already been defined.
    Iterator's constructor should produce an "end" iterator if constructed
    without parameters, otherwise a "begin" iterator. The begin constructors
    parameters should be supplied to the template, and will become the
    container's constructor parameters. Iterators don't have to be 
    const, but I think the container does, so clear() etc are disallowed. */
template< typename ITERATOR, typename ... CP >
class ContainerFromIterator : public ContainerInterface
{
public:
	typedef ITERATOR iterator; /// So that the standard Container::iterator requirement is met
	ContainerFromIterator( CP ...cp ) : my_begin( cp... ), my_end() {}
    
	virtual const iterator &begin() { return my_begin; }
    virtual const iterator &end()   { return my_end; }
    virtual void erase( const ContainerInterface::iterator_interface & ) { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    virtual void clear()                                     { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }    
    virtual const iterator &insert( const TreePtrInterface & )          { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    virtual const iterator &insert_front( const TreePtrInterface & )    { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
private:
    iterator my_begin, my_end;
};

#endif

