#ifndef ITEMISE_MEMBERS_HPP
#define ITEMISE_MEMBERS_HPP

#include <stdio.h>
#include <vector>
using namespace std;


extern char big_area[1024];

class Itemiser
{
public:
    class Itemisable
    {
    public:
        virtual ~Itemisable() {}
        Itemisable &operator=( const Itemisable &other )
        {
            if( (unsigned)this >= (unsigned)dstart &&
                (unsigned)this < (unsigned)dend )
            {
                unsigned ofs = (unsigned)this - (unsigned)dstart;
                Itemisable *wb = (Itemisable *)(bp + ofs);
                v.push_back( wb );
            }
            return *this;
        }
    };
    
    template< class C >
    static vector< Itemiser::Itemisable * > Itemise( const C *p )
    {
        static C d;
        static C s; 
        bp = (char *)p; 
        dstart = (char *)&d;
        dend = dstart + sizeof(C);
        v.clear();
        
        // This is the assignment that will be detected
        d = s;
        
        return v;     
    }

    static char *bp;
    static char *dstart;
    static char *dend;
    static vector<Itemisable *> v;
};

#define ITEMISE_FUNCTION \
    virtual vector< Itemiser::Itemisable * > Itemise() const  \
    { \
        return Itemiser::Itemise( this ); \
    } 

#endif
