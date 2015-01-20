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
/*temp*/ void *f_link;
void (f)(auto int i, auto void *link, auto char k, auto short j);
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i_1; temp_i_1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ {
 ::TopLevel::f(temp_i_1, &&LINK, temp_k, temp_j);
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_j_1; temp_j_1=(6); ({ /*temp*/ int temp_k_1; temp_k_1=(8); ({ {
 ::TopLevel::f(temp_i, &&LINK_1, temp_k_1, temp_j_1);
LINK_1:;
}
 ::TopLevel::f_return; }); }); }); });
cease( ((2)*({ /*temp*/ int temp_k_2; temp_k_2=(3); ({ /*temp*/ int temp_i_2; temp_i_2=(1); ({ /*temp*/ int temp_j_2; temp_j_2=(2); ({ {
 ::TopLevel::f(temp_i_2, &&LINK_2, temp_k_2, temp_j_2);
LINK_2:;
}
 ::TopLevel::f_return; }); }); }); }))+ ::TopLevel::x );
return ;
}

void (TopLevel::f)(int i, void *link, char k, short j)
{
/*temp*/ void *temp_link;
auto int t = i+(j*(3));
{
 ::TopLevel::f_return=(((5)*k)+t);
{
temp_link=link;
return ;
}
}
}
