

class Y
{
public:
    Y( char c )
    {
    }
};

class X
{
public:
    int i;
    Y y;
    
    X() :
        y('d')
    {
        i = 9;
    }
    
    X(int a) :
        y('a')
     
    {
        i = a;
    }
    
    ~X()
    {
        i=2;
    }
    
    void foo()
    {
    }
};

int main()
{
    X x; 
    x.foo();   
    
    X *px = new X;
    delete px;
    
    return x.i;
}
