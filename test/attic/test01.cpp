
struct Foo
{
    int h : 6;
    unsigned i : 7;
};

void ff()
{
    Foo x = { 1, 2 }; // (struct Foo){ .h=1, .i=2 }

    x = (struct Foo){ 1, 2 };
}
