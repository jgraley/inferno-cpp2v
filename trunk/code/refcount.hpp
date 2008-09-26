
#ifndef REFCOUNT_HPP
#define REFCOUNT_HPP

#include <assert.h>

template<typename TARGET>
class RCPtr;

// Reverse template
template<typename TARGET>
class RCTarget
{
public:
    RCTarget() :
        count(0)
    {
    }

    ~RCTarget()    
    {
        assert( count == 0 );
    }

private: friend class RCPtr<TARGET>;
    unsigned count;
};

template<typename TARGET>
class RCPtr
{
public:
    inline RCPtr() :
        ptr( 0 )
    {
    }

    inline RCPtr( const RCPtr &o )
    {
        ptr = o.ptr;
        Inc();
    }

    inline RCPtr &operator =(TARGET *t)
    {
        Dec();
        ptr = t;
        Inc();
        return *this;
    }
    
    inline ~RCPtr()
    {
        Dec();
    }
    
    inline TARGET *operator ->()
    {
        return ptr;
    }

private:
    TARGET *ptr;
    inline void Inc()
    {
        if( ptr )
            ((RCTarget<TARGET> *)ptr)->count++;
    }
    inline void Dec()
    {
        if( ptr )
        {
            ((RCTarget<TARGET> *)ptr)->count--;
            if( ((RCTarget<TARGET> *)ptr)->count==0 )
                delete ptr;
        }
    }
};

#endif
