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
/*temp*/ int f_return;
void (f)(auto char k, auto int i, auto short j);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_k; temp_k=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({  ::TopLevel::f(temp_k, temp_i1, temp_j);  ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_k1; temp_k1=(8); ({ /*temp*/ int temp_j1; temp_j1=(6); ({  ::TopLevel::f(temp_k1, temp_i, temp_j1);  ::TopLevel::f_return; }); }); }); });
cease( ((2)*({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_i2; temp_i2=(1); ({  ::TopLevel::f(temp_k2, temp_i2, temp_j2);  ::TopLevel::f_return; }); }); }); }))+ ::TopLevel::x );
return ;
}

void (TopLevel::f)(char k, int i, short j)
{
auto int t = ((3)*j)+i;
{
 ::TopLevel::f_return=((k*(5))+t);
return ;
}
}