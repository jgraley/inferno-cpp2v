#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
void T();
/*temp*/ int f_return;
/*temp*/ short f_j;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
/*temp*/ char f_k;
void (f)();
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

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

void (TopLevel::f)()
{
/*temp*/ void *temp_link;
auto char k =  ::TopLevel::f_k;
auto short j =  ::TopLevel::f_j;
auto void *link =  ::TopLevel::f_link;
auto int i =  ::TopLevel::f_i;
auto int t = i+(j*(3));
{
 ::TopLevel::f_return=(t+(k*(5)));
{
temp_link=link;
return ;
}
}
}
