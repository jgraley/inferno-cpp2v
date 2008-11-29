
struct { int a; } b;

class Test
{
public:
    union { int i; float f; } d;
    virtual char f( int p )
    {
        Test *tp = this;
        return p+d.i;
    }
private:
    void g();
    class Nested
    {
    };
};

// Not yet supported in clang
//void Test::g()
//{
//}


int main()
{
    Test t;
    t.d.i = 22;
    Test *pt = &t;
    int y = pt->f( 11 );
    return (unsigned char)y;
}

