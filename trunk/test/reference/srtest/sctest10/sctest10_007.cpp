#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int (f)(auto short j, auto int i, auto char k);
int x;
void T();
};
TopLevel top_level("top_level");

int (TopLevel::f)(short j, int i, char k)
{
auto int t = i+(j*(3));
return ((5)*k)+t;
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_k; temp_k=(8); ({ /*temp*/ int temp_j; temp_j=(6); ({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_j1; temp_j1=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k1; temp_k1=(0);  ::TopLevel::f(temp_j1, temp_i1, temp_k1); }); }); });  ::TopLevel::f(temp_j, temp_i, temp_k); }); }); });
cease(  ::TopLevel::x+(({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_j2; temp_j2=(2);  ::TopLevel::f(temp_j2, temp_i2, temp_k2); }); }); })*(2)) );
}
