#ifndef ITEMISE_MEMBERS_HPP
#define ITEMISE_MEMBERS_HPP

#include <stdio.h>
#include <vector>
using namespace std;


extern char big_area[1024];

class Itemiser
{
public:
    class ItemisableBase
    {
    public:
        virtual ~ItemisableBase() {}
    };
    
    template<class M>
    class Itemisable : public M, public ItemisableBase
    {
    public:
        Itemisable<M> &operator=( const Itemisable<M> &other )
        {
            //printf( "*%x=*%x big_area=%x\n", (unsigned)this, (unsigned)&other, (unsigned)big_area );
            
            if( (unsigned)this >= (unsigned)dstart &&
                (unsigned)this < (unsigned)dend )
            {
                unsigned ofs = (unsigned)this - (unsigned)dstart;
                ItemisableBase *wb = (ItemisableBase *)(bp + ofs);
                v.push_back( wb );
            }
            else
            {
                *(M*)this = *(M*)&other;
            }
            //printf("done\n");
            return *this;
        }
        Itemisable<M> &operator=( const M &other )
        {
            *(M*)this = other;
            return *this;
        }
        Itemisable()
        {
        }
        Itemisable( const M &other ) :
            M( other )
        {
        }
    };

    template< class C >
    static vector< Itemiser::ItemisableBase * > Itemise( const C *p )
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
    static vector<ItemisableBase *> v;
};

#define ITEMISE_FUNCTION \
    virtual vector< Itemiser::ItemisableBase * > Itemise() const  \
    { \
        return Itemiser::Itemise( this ); \
    } 

#endif
