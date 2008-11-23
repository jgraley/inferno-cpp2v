

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
    z = 5;
    z /= 2;
    y += (int)z;
    return (unsigned char)y;
}
