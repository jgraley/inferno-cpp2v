static struct { int a; } b;

class Test 
{
public:
    union { int i; float f; } d;
    virtual char f( int p )
    {
        //Test *tp = this;
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
	(void)f(23);
}

enum GGFF
{
   A,
   B,
   C=100,
   D
};

static struct { int c; } d;
struct BStruct { int a; };

int main()

{
	BStruct b;
    Sub t;
    t.d.i = 22;
    Test *pt = &t;
    int y = pt->f( 11 );
    y += B+D-C;
    return (unsigned char)y;
}

