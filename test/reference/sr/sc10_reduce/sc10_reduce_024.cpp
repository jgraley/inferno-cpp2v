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
int (f)(auto char k, auto short j, auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f(8, 6,  ::TopLevel::f(0, 0, 0)));
cease(  ::TopLevel::x+((2)*( ::TopLevel::f(3, 2, 1))) );
}

int (TopLevel::f)(char k, short j, int i)
{
auto int t = i+((3)*j);
return t+((5)*k);
}
