#include <stdio.h>

char big_area[1024];

class WalkableBase
{
public:
    virtual ~WalkableBase()
    {
        printf("~wb %p\n", this);
    }
};

class MemberWalkObserver
{
public:
    char *bp;
    virtual void OnWalkMember( WalkableBase *w ) = 0;
};

template<class M>
class Walkable : public M, public WalkableBase
{
public:
    Walkable<M> &operator=( const Walkable<M> &other )
    {
        printf( "op= %x %x\n", (unsigned)&other, (unsigned)big_area );
        if( (unsigned)&other >= (unsigned)big_area &&
            (unsigned)&other < (unsigned)(big_area+sizeof(big_area)) )
        {
            unsigned ofs = (unsigned)&other - (unsigned)big_area;
            MemberWalkObserver *mwo = *(MemberWalkObserver **)((char *)this - ofs);
            WalkableBase *wb = (WalkableBase *)(mwo->bp + ofs);
            mwo->OnWalkMember( wb );
        }
        else
        {
            *(M*)this = *(M*)&other;
        }
        return *this;
    }
};

#define WALK_MEMBERS_FUNCTION(C) \
    virtual void WalkMembers( MemberWalkObserver *mwo ) const \
    { \
        mwo->bp = (char *)this; \
        *(C*)&mwo = *(C*)big_area; \
    } 

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
    Walkable<Int> a;
    Walkable<Int> b;
    Walkable<Char> kjhl;
    Walkable<Int> c;
};

class myob : public MemberWalkObserver
{
    virtual void OnWalkMember( WalkableBase *w )
    {
        printf("owm %p\n", dynamic_cast< Walkable<Int> *>(w));
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
