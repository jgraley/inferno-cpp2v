
#ifndef REFCOUNT_HPP
#define REFCOUNT_HPP

#include <assert.h>

template<typename TARGET>
class RCPtr;


// Reverse template
class RCTarget
{
public:
    inline RCTarget() :
        ref_count(0)
    {
    }

    virtual inline ~RCTarget()    
    {
        assert( ref_count == 0 && "Object was explicitly destructed with references remaining");
    }

public:
//private: friend class RCPtr<typename>;
    unsigned ref_count;
};


template<typename TARGET>
class RCPtr
{
public:
    inline RCPtr() :
        ptr( 0 )
    {
    }
    
    inline RCPtr( const RCPtr<TARGET> &o )
    {
        ptr = o.ptr;
        Inc();
    }

    inline RCPtr( TARGET *t )
    {
        ptr = t;
        Inc();
    }

    template<typename OTHER_TARGET>
    inline RCPtr &operator =( const RCPtr<OTHER_TARGET> &o )
    {
        Dec();
        ptr = o.ptr;
        Inc();
        return *this;
    }
    
    inline RCPtr &operator =( TARGET *t )
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
        assert( ptr && "dereferencing null RCPtr");
        return ptr;
    }

    inline operator TARGET *()
    {
        return ptr;
    }

    TARGET *ptr;
    
private:
    inline void Inc()
    {
        if( ptr )
        {        
            ((RCTarget*)ptr)->ref_count++;
            //printf("%p++%d\n", (RCTarget*)ptr, ((RCTarget*)ptr)->ref_count);
        }
    }
    inline void Dec()
    {
        if( ptr )
        {
            assert( ((RCTarget *)ptr)->ref_count != 0 && "ref count out of sync");
            ((RCTarget *)ptr)->ref_count--;
            //printf("%p--%d\n", (RCTarget*)ptr, ((RCTarget*)ptr)->ref_count);
            if( ((RCTarget *)ptr)->ref_count==0 )
                delete ptr;
        }
    }
};

#endif
