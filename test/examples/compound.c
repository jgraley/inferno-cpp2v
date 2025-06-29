short a;
int i = sizeof(char)+sizeof(int);

int main()
{
    int x = 6;
    int y = 0;
    if( x == 6 )
    {
        if( x*2 >= 12 )
           y = 11;
    } 
    
    int as;
    for( as=0; as<3; as++ )
        y++;
    
    {
        int y; // this "y" should be a seperate variable due to scope
        y=98761;        
    }
    {
        struct y {};
        y x;
    }
    
    while( x > 3 )
    {
        x--;
        y+=10;
    }
    
    int aa=0;
    do
    {
        y = y-x;
        aa++;
    } while(aa<2);
    
    switch( x )
    {
    case 0:
    case 2:
        y=0;
        break;
    case 3:
        y+=2;
        break;
    case 5 ... 9:
        y=99;
        break;    
    default:
        y=1;
        break;
    }
    
    y ^= i; // factor in global i
    return (unsigned char)y;
}


