struct S
{
    int a;
    void f();
};

int a;
S x;
void S::f()
{
    int a;
    a=0;
}


int main()
{
    struct S
    {  
        int a;
    };
    S x;
    x.a=11;
    ::x.a=13;
    int a=3;
    ::a=7;
    ::x.f();
    return ::a + 2*a + 3*x.a + 4*::x.a;
}


struct Test
{
    void g();
};

void Test::g()
{
}
