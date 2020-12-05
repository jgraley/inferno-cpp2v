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
int (f)(auto int i, auto short j, auto char k);
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f( ::TopLevel::f(0, 0, 0), 6, 8));
cease(  ::TopLevel::x+((2)*( ::TopLevel::f(1, 2, 3))) );
}

int (TopLevel::f)(int i, short j, char k)
{
auto int t = i+((3)*j);
return t+((5)*k);
}
