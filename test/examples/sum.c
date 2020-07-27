

int main()
{
    int i;
    int t = 0;
    for( i=0; i<10; i++ )
        t += i;
    for( ; i/2<10; ++i )
    {
        t += i;
        int j;
        for( j=10; j != 15; j++ )
            if( i==j )
                t++;
    }
    return t;
}
