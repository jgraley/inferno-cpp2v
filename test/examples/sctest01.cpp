
#include <isystemc.h>
/*
class Adder : public sc_module
{
    SC_CTOR(Adder)
    {
        SC_THREAD(T);
    }
    void T();
};

class Multiplier : public sc_module
{
    SC_CTOR(Multiplier)
    {
        SC_THREAD(T);
    }
    void T();
};
*/
class TopLevel : public sc_module
{
public:
    SC_CTOR(TopLevel)
    {
        SC_THREAD(T);
    }
    void T()
    {
        exit(42);
    }
};

TopLevel top_level("top_level");

