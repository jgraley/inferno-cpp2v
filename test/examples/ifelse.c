int iftest(int p, int q)
{
    int t = 0;

    if( p )
        if( q )
            t += 1;
        else
            t += 2;
            
    if( p )
    {
        if( q )
            t += 4;
    }
    else
        t += 8;

    return t;
}
            

int main()
{
    int tt=0;
    tt += iftest(0, 0);
    tt += iftest(0, 1)*2;
    tt += iftest(1, 0)*3;
    tt += iftest(1, 1)*5;
    return tt % 128;
}

