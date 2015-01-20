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
int (f)(auto short j, auto char k, auto int i);
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i_1; temp_i_1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0);  ::TopLevel::f(temp_j, temp_k, temp_i_1); }); }); }); ({ /*temp*/ int temp_j_1; temp_j_1=(6); ({ /*temp*/ int temp_k_1; temp_k_1=(8);  ::TopLevel::f(temp_j_1, temp_k_1, temp_i); }); }); });
cease(  ::TopLevel::x+(({ /*temp*/ int temp_k_2; temp_k_2=(3); ({ /*temp*/ int temp_i_2; temp_i_2=(1); ({ /*temp*/ int temp_j_2; temp_j_2=(2);  ::TopLevel::f(temp_j_2, temp_k_2, temp_i_2); }); }); })*(2)) );
}

int (TopLevel::f)(short j, char k, int i)
{
auto int t = (j*(3))+i;
return (k*(5))+t;
}
