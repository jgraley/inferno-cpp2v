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
            if( (unsigned)&other >= (unsigned)big_area &&
                (unsigned)&other < (unsigned)(big_area+sizeof(big_area)) )
            {
                unsigned ofs = (unsigned)&other - (unsigned)big_area;
                Itemiser *i = (Itemiser *)((char *)this - ofs);
                ItemisableBase *wb = (ItemisableBase *)(i->bp + ofs);
                i->v.push_back( wb );
            }
            else
            {
                *(M*)this = *(M*)&other;
            }
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
    static vector< Itemiser::ItemisableBase * > Itemise( C *p )
    {
        Itemiser i; 
        i.bp = (char *)p; 
        *(C*)&i = *(C*)big_area; 
        return i.v;     
    }

    char *bp;
    vector<ItemisableBase *> v;
};

#define ITEMISE_FUNCTIONS() \
    virtual vector< Itemiser::ItemisableBase * > Itemise() const  \
    { \
        return Itemiser::Itemise( this ); \
    } 


/*

#define DYNAMIC_MATCH_FUNCTION(C) \
    virtual bool DynamicMatch( Node *n ) const \
    { \
        return !!dynamic_cast<C *>(n); \
    }

#define NODE_FUNCTIONS(C) WALK_MEMBERS_FUNCTION(C) DYNAMIC_MATCH_FUNCTION(C)

class Int
{
    int i;
};

class Char
{
    char i;
};

struct Node
{
    virtual ~Node()
    {
    }
};

struct Statement : Node
{
    NODE_FUNCTIONS(Statement);
};

struct Expression : Statement
{
    NODE_FUNCTIONS(Expression);
    Itemisable<Int> a;
    Itemisable<Int> b;
    Itemisable<Char> kjhl;
    Itemisable<Int> c;
};

class myob : public Itemiser
{
    virtual void OnWalkMember( ItemisableBase *w )
    {
        printf("owm %p\n", dynamic_cast< Itemisable<Int> *>(w));
    }
};

int main()
{
    Statement s;
    Expression e;
    printf("%d %d\n", e.DynamicMatch(&s), s.DynamicMatch(&e) );
    myob oo;
    Statement *spe = &e;
    spe->WalkMembers(&oo);
}
*/

#endif
