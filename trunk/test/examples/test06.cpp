
static int factorial( int n )
{
    if( n==0 )
        return 1;
    int a = n-1;
    return factorial(a) * n;
}

int main()
{
    int a = factorial(5);
    
    return (unsigned char)a;
}

