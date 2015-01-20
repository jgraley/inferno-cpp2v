#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
void T();
int (f)(auto int i, auto char k, auto short j);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f( ::TopLevel::f(0, 0, 0), 8, 6));
cease(  ::TopLevel::x+((2)*( ::TopLevel::f(1, 3, 2))) );
}

int (TopLevel::f)(int i, char k, short j)
{
auto int t = i+(j*(3));
return t+(k*(5));
}
