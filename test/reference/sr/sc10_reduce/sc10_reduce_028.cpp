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
void (f)(auto char k, auto short j, auto int i, auto void *link);
/*temp*/ int f_return;
/*temp*/ void *f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i_1; temp_i_1=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ {
 ::TopLevel::f(temp_k, temp_j, temp_i_1, &&LINK);
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_k_1; temp_k_1=(8); ({ /*temp*/ int temp_j_1; temp_j_1=(6); ({ {
 ::TopLevel::f(temp_k_1, temp_j_1, temp_i, &&LINK_1);
LINK_1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_j_2; temp_j_2=(2); ({ /*temp*/ int temp_k_2; temp_k_2=(3); ({ /*temp*/ int temp_i_2; temp_i_2=(1); ({ {
 ::TopLevel::f(temp_k_2, temp_j_2, temp_i_2, &&LINK_2);
LINK_2:;
}
 ::TopLevel::f_return; }); }); }); })) );
return ;
}

void (TopLevel::f)(char k, short j, int i, void *link)
{
/*temp*/ void *temp_link;
auto int t = i+((3)*j);
{
 ::TopLevel::f_return=(t+((5)*k));
{
temp_link=link;
return ;
}
}
}
