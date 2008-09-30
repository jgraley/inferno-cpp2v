
#ifndef REFCOUNT_HPP
#define REFCOUNT_HPP

#include <assert.h>
#include <stdio.h>

template<typename TARGET>
class RCPtr;


// Reverse template
class RCTarget
{
public:
    inline RCTarget() :
        ref_count(0),
        valid(0x11111111)
    {
        assert( started ); // disallow static allocation
    }

    virtual inline ~RCTarget()    
    {
        assert( finished || ref_count == 0 && "Object was explicitly destructed with references remaining");
        assert( valid==0x11111111 );
        valid=0xdeaddead;
    }

public:
//private: friend class RCPtr<typename>;
    unsigned ref_count;
    static void Finished() 
    {
        finished = true;
    } 
    static void Start() 
    {
        started = true;
    } 
    
//private:
    static bool started;
    static bool finished;
    
private:
    unsigned valid;    
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
    inline RCPtr( const RCPtr<OTHER_TARGET> &o )
    { 
        ptr = o.ptr;
        Inc();
    }
  
    template<typename OTHER_TARGET>
    inline RCPtr &operator =( const RCPtr<OTHER_TARGET> &o )
    {
        o.Inc(); 
        Dec();
        ptr = o.ptr;
        return *this;
    }
    
    inline RCPtr &operator =( const RCPtr<TARGET> &o )
    {
        o.Inc(); 
        Dec();
        ptr = o.ptr;
        return *this;
    }
    
    template<typename OTHER_TARGET>
    void ff( const RCPtr<OTHER_TARGET> &o )
    {
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

    inline operator bool()
    {
        return (bool)ptr;
    }

    inline TARGET &operator *()
    {
        return *ptr;
    }

    TARGET *ptr;
    
    inline void Inc() const 
    {
        if( ptr && !RCTarget::finished )
        {        
            ((RCTarget*)ptr)->ref_count++;
            //printf("%p++%d\n", (RCTarget*)ptr, ((RCTarget*)ptr)->ref_count);
        }
    }
    inline void Dec() const 
    {
        if( ptr && !RCTarget::finished )
        {
            assert( ((RCTarget *)ptr)->ref_count != 0 && "ref count out of sync");
            ((RCTarget *)ptr)->ref_count--;
            //printf("%p--%d\n", (RCTarget*)ptr, ((RCTarget*)ptr)->ref_count);
            if( ((RCTarget *)ptr)->ref_count==0 )
                delete ptr;
        }
    }
};

// A version of dynamic_cast for RCPtr. Conveniently, you only need to
// explicitly specify CANDIDATE, so the syntax is identical to 
// dynamic_cast.
template< class CANDIDATE, class BASE >
RCPtr<CANDIDATE> DynamicCast( RCPtr<BASE> p )
{
    return RCPtr<CANDIDATE>( dynamic_cast<CANDIDATE *>(p.ptr) );
}

#endif
