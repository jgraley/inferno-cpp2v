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
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ /*temp*/ int temp_j; temp_j=(0);  ::TopLevel::f(temp_i1, temp_j, temp_k); }); }); }); ({ /*temp*/ int temp_j1; temp_j1=(6); ({ /*temp*/ int temp_k1; temp_k1=(8);  ::TopLevel::f(temp_i, temp_j1, temp_k1); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3);  ::TopLevel::f(temp_i2, temp_j2, temp_k2); }); }); })) );
}

int (TopLevel::f)(int i, short j, char k)
{
auto int t = i+((3)*j);
return t+((5)*k);
}
