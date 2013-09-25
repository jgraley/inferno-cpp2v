
struct A
{
    void afunc();
    int avar;
    struct B
    {
        struct C
        {
            int cfunc( int x, int y );
            int cvar; 
        };
    };
};

void A::afunc()
{
    // avar was not being found, needed eg A::avar
    avar=5;
}

int A::B::C::cfunc( int x, int y )
{
    // check this too - it is possible for cfunc's clang::Scope 
    // to be the same as A's simply due to reallocation at the 
    // same address - danger!
    // Bring in params etc too
    cvar=2;
    return x+y;
}

A aobject;
A::B::C cobject;

int main()
{
    aobject.afunc();
    int x = cobject.cfunc(7,8);
    return aobject.avar + 2*cobject.cvar + x;   
}

