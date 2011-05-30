#ifndef RCHOLD_HPP
#define RCHOLD_HPP

#include "common/common.hpp"
#include "node/node.hpp"
#include <inttypes.h>

template<typename NODE, typename RAW>
class RCHold
{
//
// This class converts between TreePtr and void * pointers and ensures that target
// objects whoose TreePtr has been converted to void * remain in existance even if
// the number of RCPtrs to it falls to zero.
//
// Target objects will be held in existance until the corresponding RCHold object
// is destructed, after which only target objects with TreePtr refs will remain.
//
// For a set of raw pointers, create an RCHold object whose lifetime is a minimal
// superset of the union of the raw pointers' lifetimes. Do all assignment to/from
// those raw pointers using ToRaw() and FromRaw(). Note: always assign the return 
// of new/malloc to an TreePtr and then convert to a raw ptr using ToRaw() if required,
// otherwise the required extra ref will not be created.
//
// TODO: (done)
// TODO: Consider supporting the new DeleteX() virtuals in the clang action interface,
//       but most likely don't bother since we'll tear down anyway.

public:
    RCHold() :
        id( ((uintptr_t)this % 255+1) << 24 ) // 8-bit hash of this pointer, never zero, placed in top 8 bits
    {
    }
    

    RAW ToRaw( TreePtr<NODE> p )
    {
        ASSERT( p )( "Cannot convert NULL pointer to raw" );
        uintptr_t i = (unsigned)hold_list.size(); // the index of the next push_back()
        ASSERT( (i & 0xFF000000) == 0 )( "gone over maximum number of elements, probably due to infinite loop, if not rejig id" );
        i |= id; // embed an id for the current hold object  
        //TRACE("ToRaw 0x%08x\n", i );
        void *vp = reinterpret_cast<void *>( i ); 
        hold_list.push_back( p );
        ASSERT(vp); // cannot return a NULL pointer, since clang inteprets that as an error
        return vp;
    }
    
    TreePtr<NODE> FromRaw( RAW p )
    {
        ASSERT( p != 0 )( "this raw value is uninitialised");
        uintptr_t i = reinterpret_cast<uintptr_t>(p);
        //TRACE("FromRaw 0x%08x (id=0x%08x)\n", i, id );
        ASSERT( (i & 0xFF000000) == id )( "this raw value was stored to a different holder or uninitialised");
        i &= 0x00FFFFFF; 
        return hold_list[i];
    }
    
private:
    // When this object is destructed, all the members of the vector will be 
    // destructed and targets that then have no refs will be destructed.
    const unsigned id;
    vector< TreePtr< NODE > > hold_list;  // TODO does this need to be a vector?
};


#endif
