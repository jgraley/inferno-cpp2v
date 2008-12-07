char *p = "jhf";

int a[] = { 2, 3, 4 };

struct foo
{
    int h : 6;
    unsigned i : 7;
};

int func( unsigned a, char b )
{
    bool ss = true;
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
    float z;
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
    z = 5;
    z /= 2;
    y += (int)z;
    return (unsigned char)y;
}
