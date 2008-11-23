

int main()
{
    int x = 6;
    int y = 0;
    if( x == 6 )
    {
        if( x*2 >= 12 )
           y = 11;
    } 
    
    for( int a=0; a<3; a++ )
        y++;
    
    while( x > 3 )
    {
        x--;
        y+=10;
    }
    
    int a=0;
    do
    {
        y = y-x;
        a++;
    } while(a<2);
    
    switch( x )
    {
    case 0:
    case 2:
        y=0;
        break;
    case 3:
        y+=2;
        break;
    default:
        y=1;
        break;
    }
    
    return (unsigned char)y;
}

