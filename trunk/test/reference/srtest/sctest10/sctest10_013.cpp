#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void (f)();
void T();
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ void *f_link;
/*temp*/ int f_return;
/*temp*/ void *f_link_1;
int x;
/*temp*/ char f_k;
};
TopLevel top_level("top_level");

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto int i;
auto short j;
auto char k;
auto void *link;
auto int t;
k= ::TopLevel::f_k;
j= ::TopLevel::f_j;
link= ::TopLevel::f_link;
i= ::TopLevel::f_i;
t=((j*(3))+i);
{
 ::TopLevel::f_return=((k*(5))+t);
{
temp_link=link;
return ;
}
}
}

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i_1; temp_i_1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ {
{
 ::TopLevel::f_i=temp_i_1;
{
 ::TopLevel::f_link=(&&LINK);
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f();
}
}
}
}
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_j_1; temp_j_1=(6); ({ /*temp*/ int temp_k_1; temp_k_1=(8); ({ {
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_link=(&&LINK_1);
{
 ::TopLevel::f_j=temp_j_1;
{
 ::TopLevel::f_k=temp_k_1;
 ::TopLevel::f();
}
}
}
}
LINK_1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_k_2; temp_k_2=(3); ({ /*temp*/ int temp_i_2; temp_i_2=(1); ({ /*temp*/ int temp_j_2; temp_j_2=(2); ({ {
{
 ::TopLevel::f_i=temp_i_2;
{
 ::TopLevel::f_link=(&&LINK_2);
{
 ::TopLevel::f_j=temp_j_2;
{
 ::TopLevel::f_k=temp_k_2;
 ::TopLevel::f();
}
}
}
}
LINK_2:;
}
 ::TopLevel::f_return; }); }); }); })) );
return ;
}
