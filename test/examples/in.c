// Just a junk program, doesnt do anything


int g;
char *i;

typedef int mytype;
mytype vv;

int fp(int);

static int f(int &i, char longidentifier)
{
   static signed char x=9;
   unsigned char y=4;
   x++;
   return x/y;
}

int notmain()
{
    void *pl;// = &&here;
    char j=0;
    int k[7];
    {
        int a;
        a=0;
    }
    j--;
    k[4]--;
    goto here;
    here:
    j--;
    there:
    --j;
    if( k )
        goto here;
    while(88)
    {
        j++;
    }
    do
    {
        j--;
    } while(99);
    i++;
    for( *i=0, j=1; (j<10, *i<10); *i++, j++ )
    {
        *k=*i;
        break;
        continue;
    }
    for(;;)
        *k += *i;
    switch(77)
    {
        case 675:
            i++;
            j++;
            break;
        case 676:
            k[1]++;
            break;
        case 677:
        case 5:
        case 8:
        default:
            g++;
    }    
    return k[2]*4+38;
}

union ms
{
    long long int a;
    long long *b;
};

ms x;

unsigned df()
{
    x.a;
    return 0;
}

