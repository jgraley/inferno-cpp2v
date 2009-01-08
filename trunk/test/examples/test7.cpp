
struct S
{
    int a;
//    static int b;
    void f();
};

int a;
S x;
//int S::b;
void S::f()
{
    int c;
    c=0;
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
    //::S::b=19;
    return ::a + 2*a + 3*x.a + 4*::x.a;//- 2*::S::b;
    return 0;
}
