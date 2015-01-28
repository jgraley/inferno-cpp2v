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
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
void (f)();
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ {
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_link1=(&&LINK);
 ::TopLevel::f();
}
}
}
}
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_j1; temp_j1=(6); ({ /*temp*/ int temp_k1; temp_k1=(8); ({ {
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_j=temp_j1;
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_link1=(&&LINK1);
 ::TopLevel::f();
}
}
}
}
LINK1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+(({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ {
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_j=temp_j2;
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_link1=(&&LINK2);
 ::TopLevel::f();
}
}
}
}
LINK2:;
}
 ::TopLevel::f_return; }); }); }); })*(2)) );
return ;
}

void (TopLevel::f)()
{
auto int t;
auto int i;
auto short j;
auto char k;
auto void *link;
/*temp*/ void *temp_link;
link= ::TopLevel::f_link1;
k= ::TopLevel::f_k;
j= ::TopLevel::f_j;
i= ::TopLevel::f_i;
t=(i+(j*(3)));
{
 ::TopLevel::f_return=(t+(k*(5)));
{
temp_link=link;
return ;
}
}
}
