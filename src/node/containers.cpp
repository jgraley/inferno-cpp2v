#include "containers.hpp"

#include "common/common.hpp"
#include "itemise.hpp"
#include "tree_ptr.hpp"

#include <list>
#include <set>
#include <iterator>
#include <algorithm>

using namespace std;

//----------------------- ContainerInterface -------------------------

ContainerInterface::iterator_interface &ContainerInterface::iterator_interface::operator--() 
{ 
    ASSERTFAIL("Only on reversible iterator"); 
};


ContainerInterface::iterator::iterator() :
    pib( shared_ptr<iterator_interface>() ) 
{
}


ContainerInterface::iterator::iterator( const iterator_interface &ib ) 
{
    if( typeid(*this)==typeid(ib) )
        pib = dynamic_cast<const iterator &>(ib).pib; // Same type so shallow copy
    else
        pib = ib.Clone(); // Deep copy because from unmanaged source
}


ContainerInterface::iterator &ContainerInterface::iterator::operator=( const iterator_interface &ib )
{
    if( typeid(*this)==typeid(ib) )
        pib = dynamic_cast<const iterator &>(ib).pib; // Same type so shallow copy
    else
        pib = ib.Clone(); // Deep copy because from unmanaged source
    return *this;
}


ContainerInterface::iterator &ContainerInterface::iterator::operator++()
{
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    EnsureUnique();
    pib->operator++();
    return *this;
}


ContainerInterface::iterator &ContainerInterface::iterator::operator--()
{
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    EnsureUnique();
    pib->operator--();
    return *this;
}


const ContainerInterface::iterator::value_type &ContainerInterface::iterator::operator*() const 
{
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator*();
}


const ContainerInterface::iterator::value_type *ContainerInterface::iterator::operator->() const
{
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator->();
}


bool ContainerInterface::iterator::operator==( const iterator_interface &ib ) const // isovariant param
{
    if( typeid(*this)==typeid(ib) )
        return operator==(dynamic_cast<const iterator &>(ib));
    else
        return pib->operator==(ib); 
}


bool ContainerInterface::iterator::operator==( const iterator &i ) const // covariant param
{
    ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
    return pib->operator==( *(i.pib) );
}


bool ContainerInterface::iterator::operator!=( const iterator_interface &ib ) const // isovariant param
{
    return !operator==( ib );
}


bool ContainerInterface::iterator::operator!=( const iterator &i ) const // covariant param
{
    ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
    return !operator==( i );
}


void ContainerInterface::iterator::Overwrite( const value_type *v ) const
{
    ASSERT(pib)("Attempt to Overwrite through uninitialised iterator");
    pib->Overwrite( v );
}
        

const bool ContainerInterface::iterator::IsOrdered() const
{
    return pib->IsOrdered();
}


ContainerInterface::iterator_interface *ContainerInterface::iterator::GetUnderlyingIterator() const
{
    if( pib )
        return pib.get();
    else
        return nullptr;
}


shared_ptr<ContainerInterface::iterator_interface> ContainerInterface::iterator::Clone() const 
{
    return make_shared<iterator>(*this);
}


ContainerInterface::iterator::operator string()
{   
    if( pib )
        return Traceable::TypeIdName( *pib );
    else 
        return string("UNINITIALISED");
}


void ContainerInterface::iterator::EnsureUnique()
{
    // Call this before modifying the underlying iterator - Performs a deep copy
    // if required to make sure there are no other refs.
    if( pib && !pib.unique() )
        pib = pib->Clone();
    ASSERT( !pib || pib.unique() );
}


const TreePtrInterface &ContainerInterface::front()
{
    ASSERT( !empty() )("Attempting to obtain front() of an empty container");
    return *begin();
}


const TreePtrInterface &ContainerInterface::back()
{
    ASSERT( !empty() )("Attempting to obtain back() of an empty container");
    iterator t = end();
    --t;
    return *t;
}


bool ContainerInterface::empty() 
{ 
    return begin()==end(); 
}


int ContainerInterface::size() const
{
    // TODO support const_interator properly and get rid of this const_cast
    ContainerInterface *nct = const_cast<ContainerInterface *>(this);
    int n=0;
    FOREACH( const TreePtrInterface &x, *nct )
        n++;
    return n;
}
