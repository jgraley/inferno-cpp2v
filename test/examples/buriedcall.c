
int func( int z )
{
    return 5+z;
}

int main()
{ 
    int x = 8; 
    int z = 2;
    int y = x==8 ? func( z ) : 0;
    return (unsigned char)y;
}
