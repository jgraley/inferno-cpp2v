#ifndef TREE_POINTERS_HPP
#define TREE_POINTERS_HPP

#include "refcount.hpp"
#include <assert.h>

template<typename TARGET>
class OwnerPtr : public RCPtr<TARGET>
{    
public:
    inline OwnerPtr( RCTarget *n ) :
        node(n)
    {
    }
      
    template<typename OTHER_TARGET>
    inline OwnerPtr( RCTarget *n, const RCPtr<OTHER_TARGET> &p ) :
        RCPtr<TARGET>(p),
        node(n)
    {
    }  

private:    
    inline OwnerPtr() // not allowed: we always need the node
    {
        assert(!"OwnerPtr must be supplied with the containing node pointer");
    }
    RCTarget * const node;
};

template<typename TARGET>
class ShortcutPtr : public RCPtr<const TARGET>
{
public:
    ShortcutPtr()
    {
    }

    inline ShortcutPtr( const RCPtr<const TARGET> &o ) :
        RCPtr<const TARGET>(o)
    {
    }

    inline ShortcutPtr( const RCPtr<TARGET> &o )
    {
        *this = o;
    }

    template<typename OTHER>
    inline ShortcutPtr &operator =( const OTHER &o )
    {
        this->RCPtr<const TARGET>::operator=(o);
        return *this;
    }
};

#endif
