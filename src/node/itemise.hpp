#ifndef ITEMISE_HPP
#define ITEMISE_HPP

#include <stdio.h>
#include <vector>
#include "common/common.hpp"
#include <inttypes.h>

using namespace std;

// Note about multiple inheritance:
// If Itemiser::Element is at the base of a diamond, itemiser will see it twice,
// *even if* virtual inheritance is used. This may be a compiler bug in which case the above
// is true only for GCC4.3. Anyway, if we see it twice it will have the same address,
// so we de-duplicate during itemise algorithm.

class Itemiser : public virtual Traceable
{
public:
    class Element
    {
    public:
        virtual ~Element() {}
        Element &operator=( const Element &other )
        {
            if( (uintptr_t)this >= (uintptr_t)dstart &&
                (uintptr_t)this < (uintptr_t)dend )
            {
                uintptr_t ofs = (uintptr_t)this - (uintptr_t)dstart;

                FOREACH( uintptr_t x, v )
                    if( x==ofs )
                    	return *this; // don't insert if in there already, see above
                v.push_back( ofs );
                //TRACE("Itemiser caught ptr %p which is offset %d\n", this, ofs );
            }
            return *this;
        }
    };
    
	template< class ITEMISE_TYPE >
	inline static vector< uintptr_t > ItemiseImpl( const ITEMISE_TYPE *itemise_architype )
	{
		//TRACE("Static itemise %s ", typeid(*itemise_architype).name() );
		(void)itemise_architype;
		ITEMISE_TYPE d( *itemise_architype );
		ITEMISE_TYPE s( *itemise_architype );
		dstart = (char *)&d;
		dend = dstart + sizeof(d);
		v.clear();
        //TRACE("Starting itemise d=")(d)(" *arch=")(*itemise_architype)(", ptr range %p to %p\n", dstart, dend );

		// This is the assignment that will be detected
		//TRACE("Assigning ");
		d = s;
		//TRACE(" done\n");

		return v;
	}

	template< class ITEMISE_TYPE >
	inline static const vector< uintptr_t > &BasicItemiseStatic( const ITEMISE_TYPE *itemise_architype )
	{
		// Just a cache on ItemiseImpl()
		static vector< uintptr_t > v;
		static bool done=false;
		if(!done)
		{
			//TRACE("Not cached *arch=")(*itemise_architype)(", caching at %p\n", &v);
			v = ItemiseImpl( itemise_architype );
			done = true;
		}
		else
		{
			//TRACE("Using cached *arch=")(*itemise_architype)(" at %p\n", &v);
		}


		return v;
	}

	template< class ITEMISE_TYPE >
    inline static const vector< Itemiser::Element * > ItemiseStatic( const ITEMISE_TYPE *itemise_architype,
                                                                     const Itemiser *itemise_object )
    {
		//TRACE("Itemise() arch=%p *arch=", itemise_architype)(*itemise_architype)
		//               ("obj=%p *obj=", itemise_object)(*itemise_object)("\n");
        ASSERT( itemise_architype )("Itemiser got itemise_architype=NULL\n");
        ASSERT( itemise_object )("Itemiser got itemise_object=NULL\n");
        
        // Do a safety check: itemise_object we're itemising must be same as or derived
		// from the architype, so that all the architype's members are also in itemise_object.        
        ASSERT( dynamic_cast<const ITEMISE_TYPE *>(itemise_object) )
              ( "Cannot itemise because itemise_object=")(*itemise_object)
              ( " is not a nonstrict subclass of itemise_architype=")(*itemise_architype);
		
		// Do the pointer math to get "elements of A in B" type behaviour
		// This must be done in bounce because we need the architype's type for the dynamic_cast
        const vector< uintptr_t > &vofs = BasicItemiseStatic( itemise_architype );

        const ITEMISE_TYPE *target_object = dynamic_cast<const ITEMISE_TYPE *>(itemise_object);
        vector< Itemiser::Element * > vout;
        FOREACH( uintptr_t ofs, vofs )
            vout.push_back( (Element *)((const char *)target_object + ofs) );

        return vout;
    }

	template< class ITEMISE_TYPE >
	inline static Itemiser::Element *ItemiseIndexStatic( const ITEMISE_TYPE *itemise_object,
			                                             int i )
	{
		//TRACE("ItemiseIndex() index=%d obj=%p *obj=", i, itemise_object)(*itemise_object)(" size=%d\n", sizeof(*itemise_object));
		const vector< uintptr_t > &v = BasicItemiseStatic( itemise_object );
		ASSERT( i>=0 )("i=%d size=%d", i, v.size());
		ASSERT( i<v.size() )("i=%d size=%d", i, v.size());
		uintptr_t ofs = v[i];
		Element *res = (Element *)((const char *)itemise_object + ofs);
		//TRACE("ofs=%d result=%p\n", ofs, res);
		return res;
	}

	template< class ITEMISE_TYPE >
	inline static int ItemiseSizeStatic( const ITEMISE_TYPE *itemise_object )
	{
		//TRACE("ItemiseSize() obj=")(*itemise_object)("\n");
		const vector< uintptr_t > &v = BasicItemiseStatic( itemise_object );
		//TRACE("size result=%d\n", v.size());
		return v.size();
	}

	static const char *dstart;
    static const char *dend;
    static vector<uintptr_t> v;
    
    virtual vector< Itemiser::Element * > Itemise(const Itemiser *itemise_object) const = 0;
};

#define ITEMISE_FUNCTION \
    virtual vector< Itemiser::Element * > Itemise( const Itemiser *itemise_object = 0 ) const  \
    { \
        return Itemiser::ItemiseStatic( this, itemise_object ? itemise_object : this ); \
    } \
    virtual Itemiser::Element *ItemiseIndex( int i ) const  \
    { \
        return Itemiser::ItemiseIndexStatic( this, i ); \
    } \
	virtual int ItemiseSize() const  \
	{ \
		return Itemiser::ItemiseSizeStatic( this ); \
	}
#endif