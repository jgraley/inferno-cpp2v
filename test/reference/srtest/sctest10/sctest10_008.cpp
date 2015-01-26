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
int (f)(auto char k, auto int i, auto short j);
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_k; temp_k=(8); ({ /*temp*/ int temp_j; temp_j=(6); ({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_j1; temp_j1=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k1; temp_k1=(0);  ::TopLevel::f(temp_k1, temp_i1, temp_j1); }); }); });  ::TopLevel::f(temp_k, temp_i, temp_j); }); }); });
cease( (({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_j2; temp_j2=(2);  ::TopLevel::f(temp_k2, temp_i2, temp_j2); }); }); })*(2))+ ::TopLevel::x );
return ;
}

int (TopLevel::f)(char k, int i, short j)
{
auto int t = ((3)*j)+i;
return (k*(5))+t;
}
