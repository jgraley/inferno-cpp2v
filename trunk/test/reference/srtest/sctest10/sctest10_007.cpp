#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int (f)(auto char k, auto int i, auto short j);
int x;
void T();
};
TopLevel top_level("top_level");

int (TopLevel::f)(char k, int i, short j)
{
auto int t = ((3)*j)+i;
return (k*(5))+t;
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_k; temp_k=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_j; temp_j=(0);  ::TopLevel::f(temp_k, temp_i1, temp_j); }); }); }); ({ /*temp*/ int temp_k1; temp_k1=(8); ({ /*temp*/ int temp_j1; temp_j1=(6);  ::TopLevel::f(temp_k1, temp_i, temp_j1); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_i2; temp_i2=(1);  ::TopLevel::f(temp_k2, temp_i2, temp_j2); }); }); })) );
}
