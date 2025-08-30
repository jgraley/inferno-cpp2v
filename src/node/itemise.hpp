#ifndef ITEMISE_HPP
#define ITEMISE_HPP

#include "common/common.hpp"
#include "common/trace.hpp"

#include <inttypes.h>
#include <stdio.h>
#include <vector>

using namespace std;

// Note about multiple inheritance:
// If Itemiser::Element is at the base of a diamond, itemiser will see it twice,
// *even if* virtual inheritance is used. This may be a compiler bug in which case the above
// is true only for GCC4.3. Anyway, if we see it twice it will have the same address,
// so we de-duplicate during itemise algorithm.
/// Support class allowing the child-pointing members of a node to be extracted in a vector
class Itemiser : public virtual Traceable
{
public:
    class Element : public virtual Traceable
    {
    public:
		Element() = default;
        virtual ~Element() {}        
        Element(const Itemiser::Element&) = default;
        Element &operator=( const Element & )
        {
            if( (uintptr_t)this >= (uintptr_t)dstart &&
                (uintptr_t)this < (uintptr_t)dend )
            {
                uintptr_t ofs = (uintptr_t)this - (uintptr_t)dstart;

                for( uintptr_t x : v )
                    if( x==ofs )
                        return *this; // don't insert if in there already, see above
                v.push_back( ofs );
                //TRACE("Itemiser caught ptr %p which is offset %d\n", this, ofs );
            }
            return *this;
        }        
    };
    
    template< class ITEMISE_TYPE >
    inline static vector< uintptr_t > ItemiseImpl( const ITEMISE_TYPE *itemise_archetype )
    {
        //TRACES("Static itemise %s ", typeid(*itemise_archetype).name() );
        (void)itemise_archetype;
        ITEMISE_TYPE d( *itemise_archetype );
        ITEMISE_TYPE s( *itemise_archetype );
        dstart = (char *)&d;
        dend = dstart + sizeof(d);
        v.clear();
        //TRACES("Starting itemise d=")(d)(" *arch=")(*itemise_archetype)(", ptr range %p to %p\n", dstart, dend );

        // This is the assignment that will be detected
        //TRACES("Assigning ");
        d = s;
        //TRACES(" done\n");

        return v;
    }

    template< class ITEMISE_TYPE >
    inline static const vector< uintptr_t > &BasicItemiseStatic( const ITEMISE_TYPE *itemise_archetype )
    {
        // Just a cache on ItemiseImpl()
        static thread_local vector< uintptr_t > v;
        static thread_local bool done=false;
        if(!done)
        {
            //TRACES("Not cached *arch=")(*itemise_archetype)(", caching at %p\n", &v);
            v = ItemiseImpl( itemise_archetype );
            done = true;
        }
        else
        {
            //TRACES("Using cached *arch=")(*itemise_archetype)(" at %p\n", &v);
        }


        return v;
    }

    template< class ITEMISE_TYPE >
    inline static const vector< Itemiser::Element * > ItemiseStatic( const ITEMISE_TYPE *itemise_archetype,
                                                                     const Itemiser *itemise_object )
    {
        //TRACES("Itemise() arch=%p *arch=", itemise_archetype)(*itemise_archetype)
        //      ("obj=%p *obj=", itemise_object)(*itemise_object)("\n");
        ASSERTS( itemise_archetype )("Itemiser got itemise_archetype=nullptr\n");
        ASSERTS( itemise_object )("Itemiser got itemise_object=nullptr\n");
        
        // Do a safety check: itemise_object we're itemising must be same as or derived
        // from the archetype, so that all the archetype's members are also in itemise_object.        
        ASSERTS( dynamic_cast<const ITEMISE_TYPE *>(itemise_object) )
               ( "Cannot itemise because itemise_object=")(*itemise_object)
               ( " is not a nonstrict subclass of itemise_archetype=")(*itemise_archetype);
        
        // Do the pointer math to get "elements of A in B" type behaviour
        // This must be done in bounce because we need the archetype's type for the dynamic_cast
        const vector< uintptr_t > &vofs = BasicItemiseStatic( itemise_archetype );

        const ITEMISE_TYPE *target_object = dynamic_cast<const ITEMISE_TYPE *>(itemise_object);
        vector< Itemiser::Element * > vout;
        for( uintptr_t ofs : vofs )
            vout.push_back( (Element *)((const char *)target_object + ofs) );

        return vout;
    }

    template< class ITEMISE_TYPE >
    inline static Itemiser::Element *ItemiseIndexStatic( const ITEMISE_TYPE *itemise_object,
                                                         vector< uintptr_t >::size_type i )
    {
        //TRACE("ItemiseIndex() index=%d obj=%p *obj=", i, itemise_object)(*itemise_object)(" size=%d\n", sizeof(*itemise_object));
        const vector< uintptr_t > &v = BasicItemiseStatic( itemise_object );
        ASSERTS( i>=0 )("i=%d size=%d", i, v.size());
        ASSERTS( i<v.size() )("i=%d size=%d", i, v.size());
        uintptr_t ofs = v[i];
        Element *res = (Element *)((const char *)itemise_object + ofs);
        //TRACE("ofs=%d result=%p\n", ofs, res);
        return res;
    }

    template< class ITEMISE_TYPE >
    inline static int ItemiseSizeStatic( const ITEMISE_TYPE *itemise_object )
    {
        //TRACES("ItemiseSize() obj=")(*itemise_object)("\n");
        const vector< uintptr_t > &v = BasicItemiseStatic( itemise_object );
        //TRACES("size result=%d\n", v.size());
        return v.size();
    }

    static thread_local const char *dstart;
    static thread_local const char *dend;
    static thread_local vector<uintptr_t> v;
    
    virtual vector< Itemiser::Element * > Itemise(const Itemiser *itemise_object) const = 0;
};

#define ITEMISE_FUNCTION \
    virtual vector< Itemiser::Element * > Itemise( const Itemiser *itemise_object = 0 ) const  \
    { \
        return Itemiser::ItemiseStatic( this, itemise_object ? itemise_object : this ); \
    } \
    virtual Itemiser::Element *ItemiseIndex( vector< uintptr_t >::size_type i ) const  \
    { \
        return Itemiser::ItemiseIndexStatic( this, i ); \
    } \
    virtual int ItemiseSize() const  \
    { \
        return Itemiser::ItemiseSizeStatic( this ); \
    }
#endif
