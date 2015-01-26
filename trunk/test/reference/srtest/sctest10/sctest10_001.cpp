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
int (f)(auto char k, auto int i, auto short j);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=( ::TopLevel::f(8,  ::TopLevel::f(0, 0, 0), 6));
cease( (( ::TopLevel::f(3, 1, 2))*(2))+ ::TopLevel::x );
}

int (TopLevel::f)(char k, int i, short j)
{
auto int t = i+(j*(3));
return t+(k*(5));
}
