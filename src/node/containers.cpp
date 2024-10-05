#include "containers.hpp"

#include "common/common.hpp"
#include "itemise.hpp"
#include "tree_ptr.hpp"

#include <list>
#include <set>
#include <iterator>
#include <algorithm>

#define CHECK_NOT_REACHED_ON_SUBCLASS \
    ASSERT(typeid(*this)==typeid(decltype(*this))) \
          ("Call to proxy iterator method on (presumably concrete) iterator object.\n") \
          ("Called on ")(typeid(*this))("\n") \
          ("Proxy class is ")(typeid(decltype(*this)))("\n"); \

using namespace std;

//----------------------- ContainerInterface -------------------------

ContainerInterface::iterator_interface &ContainerInterface::iterator_interface::operator--() 
{ 
    ASSERTFAIL("Only on reversible iterator"); 
};


ContainerInterface::iterator::iterator() :
    pib( unique_ptr<iterator_interface>() ) 
{
    //FTRACE("%p Cons noarg\n", this);
}


ContainerInterface::iterator::iterator( const iterator &ib ) :
    ContainerInterface::iterator::iterator( (const iterator_interface &)ib )
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    // This "big three" copy constructor just delegates to the iterator_interface
    // constructor. Without this we'd get the default copy and our attempts
    // to manage pib would go awry.
}


ContainerInterface::iterator &ContainerInterface::iterator::operator=( const iterator &ib )
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    // This "big three" assign operator just delegates to the iterator_interface
    // assign operator. Without this we'd get the default assign and our attempts
    // to manage pib would go awry.
    return operator=( (const iterator_interface &)ib );
}


ContainerInterface::iterator::~iterator()
{
}


ContainerInterface::iterator::iterator( const iterator_interface &ib ) 
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    
    // We always deep-copy immediately (no attempt at copy-on-write etc)    
    if( typeid(*this)==typeid(ib) )
    {
        // ib is the exact same type as us. We can make ourself equivalent 
        // to ib, but we still have to clone ib's pib. Adds 0 layers of
        // indirection.
        const unique_ptr<iterator_interface> &ib_pib = dynamic_cast<const iterator &>(ib).pib;
        pib = ib_pib ? ib_pib->Clone() : nullptr;
    }
    else
    {
        // ib is a subclass of iterator_interface but not the same as us, so 
        // the only safe thing to do is to clone ib. Adds 1 layer of indirection.
        pib = ib.Clone(); 
    }
}


ContainerInterface::iterator &ContainerInterface::iterator::operator=( const iterator_interface &ib )
{
    CHECK_NOT_REACHED_ON_SUBCLASS

    if( typeid(*this)==typeid(ib) )
    {
        // ib is the exact same type as us. We can make ourself equivalent 
        // to ib, but we still have to clone ib's pib. Adds 0 layers of
        // indirection.
        const unique_ptr<iterator_interface> &b_pib = dynamic_cast<const iterator &>(ib).pib;
        pib = b_pib ? b_pib->Clone() : nullptr;
    }
    else
    {
        // ib is a subclass of iterator_interface but not the same as us, so 
        // the only safe thing to do is to clone ib. Adds 1 layer of indirection.
        pib = ib.Clone();
    }
    return *this;
}


ContainerInterface::iterator &ContainerInterface::iterator::operator++()
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    pib->operator++();
    return *this;
}


ContainerInterface::iterator &ContainerInterface::iterator::operator--()
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    ASSERT(pib)("Attempt to increment uninitialised iterator");
    pib->operator--();
    return *this;
}


const ContainerInterface::iterator::value_type &ContainerInterface::iterator::operator*() const 
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator*();
}


const ContainerInterface::iterator::value_type *ContainerInterface::iterator::operator->() const
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    ASSERT(pib)("Attempt to dereference uninitialised iterator");
    return pib->operator->();
}


bool ContainerInterface::iterator::operator==( const iterator_interface &ib ) const // isovariant param
{
    CHECK_NOT_REACHED_ON_SUBCLASS

    if( typeid(*this)==typeid(ib) )
    {
        const unique_ptr<iterator_interface> &ib_pib = dynamic_cast<const iterator &>(ib).pib;
        ASSERT(pib && ib_pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", ib_pib?"i":"U");
        return pib->operator==(*ib_pib);
    }
    else
    {
        return pib->operator==(ib); 
    }
}


bool ContainerInterface::iterator::operator==( const iterator &i ) const // covariant param
{
    CHECK_NOT_REACHED_ON_SUBCLASS

    if( typeid(*this)==typeid(i) )
    {
        const unique_ptr<iterator_interface> &i_pib = dynamic_cast<const iterator &>(i).pib;
        ASSERT(pib && i_pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i_pib?"i":"U");
        return pib->operator==(*i_pib);
    }
    else
    {
        return pib->operator==(i); 
    }
}


bool ContainerInterface::iterator::operator!=( const iterator_interface &ib ) const // isovariant param
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    return !operator==( ib );
}


bool ContainerInterface::iterator::operator!=( const iterator &i ) const // covariant param
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    return !operator==( i );
}


void ContainerInterface::iterator::Overwrite( const value_type *v ) const
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    ASSERT(pib)("Attempt to Overwrite through uninitialised iterator");
    pib->Overwrite( v );
}
        

const bool ContainerInterface::iterator::IsOrdered() const
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    return pib->IsOrdered();
}


ContainerInterface::iterator_interface *ContainerInterface::iterator::GetUnderlyingIterator() const
{
    CHECK_NOT_REACHED_ON_SUBCLASS
    if( pib )
        return pib.get();
    else
        return nullptr;
}


unique_ptr<ContainerInterface::iterator_interface> ContainerInterface::iterator::Clone() const 
{
    CHECK_NOT_REACHED_ON_SUBCLASS

    return make_unique<iterator>(*this);
}


ContainerInterface::iterator::operator string() const
{   
    CHECK_NOT_REACHED_ON_SUBCLASS
    if( pib )
        return Traceable::TypeIdName( *pib );
    else 
        return string("UNINITIALISED");
}


ContainerInterface::iterator::operator bool() const 
{ 
    CHECK_NOT_REACHED_ON_SUBCLASS 
    return !!pib; 
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


const ContainerInterface::iterator &ContainerInterface::erase2( const iterator &it ) // TODO lose the 2, same for erase()
{
    return erase( *it.GetUnderlyingIterator() );
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
    for( const TreePtrInterface &x : *nct )
        n++;
    return n;
}
