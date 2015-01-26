#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (f)(auto int i, auto char k, auto short j);
/*temp*/ int f_return;
int x;
void T();
};
TopLevel top_level("top_level");

void (TopLevel::f)(int i, char k, short j)
{
auto int t = i+(j*(3));
{
 ::TopLevel::f_return=(t+(k*(5)));
return ;
}
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_k; temp_k=(8); ({ /*temp*/ int temp_j; temp_j=(6); ({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_j1; temp_j1=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k1; temp_k1=(0); ({  ::TopLevel::f(temp_i1, temp_k1, temp_j1);  ::TopLevel::f_return; }); }); }); }); ({  ::TopLevel::f(temp_i, temp_k, temp_j);  ::TopLevel::f_return; }); }); }); });
cease( (({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_j2; temp_j2=(2); ({  ::TopLevel::f(temp_i2, temp_k2, temp_j2);  ::TopLevel::f_return; }); }); }); })*(2))+ ::TopLevel::x );
return ;
}
