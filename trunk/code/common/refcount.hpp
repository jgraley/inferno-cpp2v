
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

#include <boost/shared_ptr.hpp>

template<class TARGET>
class RCPtr : public boost::shared_ptr<TARGET>
{
public:
    inline RCPtr() :
        boost::shared_ptr<TARGET>()
    { 
    }
    template<class OTHER_TARGET>
    inline RCPtr( const RCPtr<OTHER_TARGET> &o ) :
        boost::shared_ptr<TARGET>( boost::static_pointer_cast<TARGET>(o) )
    { 
    }
    template<class OTHER_TARGET>
    inline RCPtr( OTHER_TARGET *o ) :
        boost::shared_ptr<TARGET>( o )
    { 
    }
    inline RCPtr( boost::shared_ptr<TARGET> p ) :
        boost::shared_ptr<TARGET>(p)
    { 
    }
    template<class BASE >
    static RCPtr Specialise( RCPtr<BASE> p )
    {
        assert( p && "Input to Specialise<>() must be non-NULL"); // since we return NULL when the cast isn't possible
        return RCPtr( boost::dynamic_pointer_cast<TARGET>(p) );
    }
    
};

template<typename NODE, typename RAW>
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
//
// TODO: Template the class on the RCPtr target *and* the raw type. And lose
// the specialise/dynamic_cast
public:
    RCHold() :
        id( ((unsigned)this % 255) << 24 )
    {
    }
    

    RAW ToRaw( RCPtr<NODE> p )
    {
        unsigned i = (unsigned)hold_list.size(); // the index of the next push_back()
        assert( (i & 0xFF000000) == 0 && "gone over maximum number of elements, probably due to infinite loop, if not rejig id" );
        i |= id; // embed an id for the current hold object  
        void *vp = reinterpret_cast<void *>( i ); 
        hold_list.push_back( p );
        return vp;
    }
    
    RCPtr<NODE> FromRaw( RAW p )
    {
        unsigned i = reinterpret_cast<unsigned>(p);
        assert( (i & 0xFF000000) == id && "this raw value was stored to a different holder");
        i &= 0x00FFFFFF; 
        return hold_list[i];
    }
    
private:
    // When this object is destructed, all the members of the vector will be 
    // destructed and targets that then have no refs will be destructed.
    const unsigned id;
    std::vector< RCPtr< NODE > > hold_list;  // TODO does this need to be a vector?
};

#endif
