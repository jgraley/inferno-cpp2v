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
    
	const iterator &begin() final { return my_begin; }
    const iterator &end() final   { return my_end; }
    const iterator &erase1( const ContainerInterface::iterator_interface & ) final { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    void clear() final                                     { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }    
    const iterator &insert( const TreePtrInterface & ) final          { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    const iterator &insert( const ContainerInterface::iterator_interface &pos, 
                            const TreePtrInterface & ) final          { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    const iterator &insert_front( const TreePtrInterface & ) final    { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
private:
    iterator my_begin, my_end;
};

#endif

