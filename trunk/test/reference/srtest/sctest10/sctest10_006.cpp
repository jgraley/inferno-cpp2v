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
int (f)(auto int i, auto short j, auto char k);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f( ::TopLevel::f(0, 0, 0), 6, 8));
cease(  ::TopLevel::x+(( ::TopLevel::f(1, 2, 3))*(2)) );
}

int (TopLevel::f)(int i, short j, char k)
{
auto int t = i+(j*(3));
return t+(k*(5));
}
