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

class Other
{
};

class Sub : public Test, virtual private Other
{
};

void Test::g()
{
}

enum GGFF
{
   A,
   B,
   C=100,
   D
};

struct { int c; } d;

int main()
{
    struct { int a; } b;
    Sub t;
    t.d.i = 22;
    Test *pt = &t;
    int y = pt->f( 11 );
    y += B+D-C;
    return (unsigned char)y;
}

