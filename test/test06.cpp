
static int factorial( int n )
{
    if( n==0 )
    { // TODO stack generation goes wrong for return not in compound statement
        return 1;
    }
    int a = n-1;
    int b = factorial(a) * n;
    return b;
}

int main()
{
    int a = factorial(5);
    
    return (unsigned char)a;
}

