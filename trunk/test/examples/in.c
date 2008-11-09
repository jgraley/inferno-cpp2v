int g;
char *i;

typedef int mytype;
mytype vv;

int fp(int);

int f(int &i, char longidentifier)
{
   signed char x=i;
   unsigned char y=4;
   x++;
   return x/y;
}

int main()
{
    void *pl;// = &&here;
    char j=0;
    int k[7] = f(9, j);
    {
        int a;
        a=0;
    }
    k--;
    k[4]--;
    goto here;
    here:
    there:
    --k;
    if( k )
        goto here;
    while(88)
    {
        j++;
    }
    do
    {
        k--;
    } while(99);
    i++;
    for( i=0, j=1; (j<10, i<10); i++, j++ )
    {
        k=i;
        break;
        continue;
    }
    for(;;)
        k+=i;
    switch(77)
    {
        case 675:
            i++;
        default:
            j++;
        case 676:
            k++;
        default:
        case 677:
        case 5:
        case 8:
        default:
        default:
            g++;
    }    
    return k*4+38;
}

union ms
{
    long long int a;
    long long *b;
};

ms x;

unsigned f()
{
    x.a = 9;
}

class ccc
{
    ms asasas;
    typedef short ti;
public:
    int ggh;
    struct nested
    {    
        ccc *p;
    };
protected:
    int a;
};

