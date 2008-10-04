
#ifndef REFCOUNT_HPP
#define REFCOUNT_HPP

#include <assert.h>
#include <stdio.h>
#include <vector>

template<class TARGET>
class RCPtr;


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
//private: friend class RCPtr<class>; // not private due to problems friending across template instantations
    unsigned ref_count;
    static void Finished() 
    {
        finished = true;
    } 
    static void Start() 
    {
        started = true;
    } 
    
//private:// not private due to problems friending across template instantations
    static bool started;
    static bool finished;
    
private:
    unsigned valid;    
};


template<class TARGET>
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

    template<class OTHER_TARGET>
    inline RCPtr( const RCPtr<OTHER_TARGET> &o )
    { 
        ptr = o.ptr;
        Inc();
    }
  
    template<class OTHER_TARGET>
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
    
    template<class OTHER_TARGET>
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

    TARGET *ptr; // not private due to problems friending across template instantations
    
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

    // A version of dynamic_cast for RCPtr. Conveniently, you only need to
    // explicitly specify CANDIDATE, so the syntax is identical to 
    // dynamic_cast.
    template<class BASE >
    static RCPtr Specialise( RCPtr<BASE> p )
    {
        return RCPtr<TARGET>( dynamic_cast<TARGET *>(p.ptr) );
    }
};


class RCHold
{
//
// This class converts between RCPtr and void * pointers and ensures that target
// objects whoose RCPtr has been converted to void * remain in existance even if
// the number of RCPtrs to it falls to zero.
//
// Target objects will be held in existance until the corresponding RCHold object
// is destructed, after which only target objects with RCPtr refs will remain.
//
// For a set of raw pointers, create an RCHold object whose lifetime is a minimal
// superset of the union of the raw pointers' lifetimes. Do all assignment to/from
// those raw pointers using ToRaw() and FromRaw(). Note: always assign the return 
// of new/malloc to an RCPtr and then convert to a raw ptr using ToRaw() if required,
// otherwise the required extra ref will not be created.
#define CHECK_POINTERS
public:
    template<class TARGET>
    void *ToRaw( RCPtr<TARGET> p )
    {
        RCPtr<RCTarget> bp(p.ptr); // Genericise the target since we can only store one type
        hold_list.push_back( bp );
        void *vp = reinterpret_cast<void *>( p.ptr );
#ifdef CHECK_POINTERS
        assert( ((unsigned)vp & 3) == 0 );
        vp = (void *)((unsigned)vp | 3);
#endif        
        return vp;
    }
    
    template<class TARGET>
    RCPtr<TARGET> FromRaw( void *p )
    {
        void *vp = p;
#ifdef CHECK_POINTERS
        assert( ((unsigned)vp & 3) == 3 );
        vp = (void *)((unsigned)vp & ~3);
#endif             
        return RCPtr<TARGET>( reinterpret_cast<TARGET *>( vp ) );
    }
    
private:
    // When this object is destructed, all the members of the vector will be 
    // destructed and targets that then have no refs will be destructed.
    std::vector< RCPtr< RCTarget > > hold_list;  // TODO does this need to be a vector?
};

#endif
