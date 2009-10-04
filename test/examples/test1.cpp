int a[] = { 2, 3, 4 };

void f()
{
    *(a+1);
}

const char *p = "jhf";

void ff()
{
    struct Foo
    {
        int h : 6;
        unsigned i : 7;
    };

    Foo x = { 1, 2 }; // (struct Foo){ .h=1, .i=2 }

    x = (struct Foo){ 1, 2 };
}

int func( unsigned a, char b )
{
    bool ss = true;
    unsigned g=a;
    if(ss)
        return 5+a*2+b;
    else
        return 0;
}

int main()
{
    short x = 6;
    unsigned long long y = func( x, 9 );
    y += (x==6) ? 2 : 3;
    -1U; // wrong
    4000000000; // wrong (>2^31)
    2000000000;
    4000000000U;
    -2000000000;
    3LL;
    1.4;
    5.6f;
    7.7l;
    y += 'z';
    y -= p[2];    
    {
        float z;
        z = 5;
        z /= 2;
        y += (int)z;
    }
    int kk=0;
    y += kk;
    return (unsigned char)y;
}

