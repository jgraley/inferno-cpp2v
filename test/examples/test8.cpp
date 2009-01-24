

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
/*
struct P 
{
    virtual void f()=0;
};

struct Q : P
{
    virtual void f() {}
};
*/
int main()
{
    X x; 
    x.foo();   
    
    X *px = new X(7);
    delete px;
    
    X *pax = ::new X[66];
    ::delete[] pax;
    return x.i;
}
