#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (f)(auto int i, auto short j, auto char k, auto void *link);
void T();
int x;
/*temp*/ int f_return;
/*temp*/ void *f_link;
};
TopLevel top_level("top_level");

void (TopLevel::f)(int i, short j, char k, void *link)
{
/*temp*/ void *temp_link;
auto int t = i+(j*(3));
{
 ::TopLevel::f_return=(t+(k*(5)));
{
temp_link=link;
return ;
}
}
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ {
 ::TopLevel::f(temp_i1, temp_j, temp_k, &&LINK);
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_j1; temp_j1=(6); ({ /*temp*/ int temp_k1; temp_k1=(8); ({ {
 ::TopLevel::f(temp_i, temp_j1, temp_k1, &&LINK1);
LINK1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+(({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ {
 ::TopLevel::f(temp_i2, temp_j2, temp_k2, &&LINK2);
LINK2:;
}
 ::TopLevel::f_return; }); }); }); })*(2)) );
return ;
}
