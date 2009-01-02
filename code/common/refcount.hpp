#ifndef REFCOUNT_HPP
#define REFCOUNT_HPP

#include "common.hpp"

template<typename NODE, typename RAW>
class RCHold
{
//
// This class converts between shared_ptr and void * pointers and ensures that target
// objects whoose shared_ptr has been converted to void * remain in existance even if
// the number of RCPtrs to it falls to zero.
//
// Target objects will be held in existance until the corresponding RCHold object
// is destructed, after which only target objects with shared_ptr refs will remain.
//
// For a set of raw pointers, create an RCHold object whose lifetime is a minimal
// superset of the union of the raw pointers' lifetimes. Do all assignment to/from
// those raw pointers using ToRaw() and FromRaw(). Note: always assign the return 
// of new/malloc to an shared_ptr and then convert to a raw ptr using ToRaw() if required,
// otherwise the required extra ref will not be created.
//
// TODO: Template the class on the shared_ptr target *and* the raw type. And lose
// the specialise/dynamic_cast
public:
    RCHold() :
        id( ((unsigned)this % 255+1) << 24 ) // 8-bit hash of this pointer, never zero, placed in top 8 bits
    {
    }
    

    RAW ToRaw( shared_ptr<NODE> p )
    {
        ASSERT( p && "Cannot convert NULL pointer to raw" );
        unsigned i = (unsigned)hold_list.size(); // the index of the next push_back()
        ASSERT( (i & 0xFF000000) == 0 && "gone over maximum number of elements, probably due to infinite loop, if not rejig id" );
        i |= id; // embed an id for the current hold object  
        //TRACE("ToRaw 0x%08x\n", i );
        void *vp = reinterpret_cast<void *>( i ); 
        hold_list.push_back( p );
        ASSERT(vp); // cannot return a NULL pointer, since clang inteprets that as an error
        return vp;
    }
    
    shared_ptr<NODE> FromRaw( RAW p )
    {
        ASSERT( p != 0 && "this raw value is uninitialised");        
        unsigned i = reinterpret_cast<unsigned>(p);
        //TRACE("FromRaw 0x%08x (id=0x%08x)\n", i, id );
        ASSERT( (i & 0xFF000000) == id && "this raw value was stored to a different holder or uninitialised");
        i &= 0x00FFFFFF; 
        return hold_list[i];
    }
    
private:
    // When this object is destructed, all the members of the vector will be 
    // destructed and targets that then have no refs will be destructed.
    const unsigned id;
    vector< shared_ptr< NODE > > hold_list;  // TODO does this need to be a vector?
};

#endif
