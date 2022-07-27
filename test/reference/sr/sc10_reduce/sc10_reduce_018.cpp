#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
int x;
int (f)(auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
cease(  ::TopLevel::f(1) );
}

int (TopLevel::f)(int i)
{
return i;
}
