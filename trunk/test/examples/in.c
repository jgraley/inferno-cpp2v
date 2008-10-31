int g;
int *i;

typedef int mytype;
mytype vv;

int fp(int);

int f(int &i, char longidentifier)
{
   int x=i;
   int y=4-1;
   x++;
   return x/y;
}

int main()
{
    void *pl;// = &&here;
    char j=0;
    int k = f(9, j);
    {
        int a;
        a=0;
    }
    k--;
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
    int a;
    char *b;
};

class ccc
{
    ms asasas;
    typedef int ti;
    struct nested
    {    
        ccc *p;
    };
    int a;
};

ms x;
