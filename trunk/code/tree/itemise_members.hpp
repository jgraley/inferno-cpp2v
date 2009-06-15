#ifndef ITEMISE_MEMBERS_HPP
#define ITEMISE_MEMBERS_HPP

#include <stdio.h>
#include <vector>
#include "common/common.hpp"

using namespace std;

// Note about multiple inheritance:
// If Itemiser::Element is at the base of a diamond, itemiser will see it twice,
// *even if* virtual inheritance is used. This may be a compiler big in which case the above
// is true only for GCC4.3. Anyway, if we see it twice it will have the same address,
// so we de-duplicate during itemise algorithm.

class Itemiser
{
public:
    class Element
    {
    public:
        virtual ~Element() {}
        Element &operator=( const Element &other )
        {
            if( (unsigned)this >= (unsigned)dstart &&
                (unsigned)this < (unsigned)dend )
            {
                unsigned ofs = (unsigned)this - (unsigned)dstart;
                Element *wb = (Element *)(bp + ofs);
                FOREACH( Element *x, v )
                    if( x==wb )
                    	return *this; // don't insert if in there already, see above
                v.push_back( wb );
            }
            return *this;
        }
    };
    
    template< class ITEMISE_TYPE >
    inline static vector< Itemiser::Element * > ItemiseStatic( const ITEMISE_TYPE *itemise_architype,
                                                                 const Itemiser *itemise_object )
    {
        (void)itemise_architype; // don't care about value of architypes; just want the type
        static ITEMISE_TYPE d;
        static ITEMISE_TYPE s; 
        bp = (const char *)dynamic_cast<const ITEMISE_TYPE *>(itemise_object); 
        dstart = (char *)&d;
        dend = dstart + sizeof(d);
        v.clear();
        
        // This is the assignment that will be detected
        d = s;
        
        return v;     
    }

    static const char *bp;
    static const char *dstart;
    static const char *dend;
    static vector<Element *> v;
    
    static vector< Itemiser::Element * > Itemise( shared_ptr<Itemiser> itemise_object, 
                                                  shared_ptr<Itemiser> limit_class_architype )
    {
        return Itemise( itemise_object.get(), limit_class_architype.get() );
    }                                                  

    static vector< Itemiser::Element * > Itemise( shared_ptr<Itemiser> itemise_object )
    {
        return Itemise( itemise_object.get() );
    }                                                  

    static vector< Itemiser::Element * > Itemise( const Itemiser *itemise_object, 
                                                  const Itemiser *limit_class_architype = 0 )
    {
        if( limit_class_architype )
            return limit_class_architype->ItemiseVirtual( itemise_object ); // Only itemise members of itemise_object that are in the superclass of limit_class_architype
        else
            return itemise_object->ItemiseVirtual( itemise_object );        // Itemise all members of itemise_object
    }    
    
    virtual vector< Itemiser::Element * > ItemiseVirtual(const Itemiser *itemise_object) const = 0;
};

#define ITEMISE_FUNCTION \
	private: friend class Itemiser; \
    virtual vector< Itemiser::Element * > ItemiseVirtual( const Itemiser *itemise_object ) const  \
    { \
        return Itemiser::ItemiseStatic( this, itemise_object ); \
    } \
    public:

#endif
