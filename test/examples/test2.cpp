

int func( unsigned **a, char *b, int &c )
{
    return 5+**a*2+b[1]+c;
    a++;
    *a++;
    b++;
    c++;
}

int main()
{
    unsigned x = 6;
    char xx[4];
    xx[2] = 7;
    xx[1] = 0;
    xx[3] = 0;
    unsigned *px = &x;
    int z=2;
    int y = func( &px, xx, z );
    y+=z;
    return (unsigned char)y;
}
