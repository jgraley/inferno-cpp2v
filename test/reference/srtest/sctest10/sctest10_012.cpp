#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
int x;
/*temp*/ short f_j;
void T();
/*temp*/ char f_k;
void (f)();
/*temp*/ int f_i;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_k; temp_k=(8); ({ /*temp*/ int temp_j; temp_j=(6); ({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_j1; temp_j1=(0); ({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k1; temp_k1=(0); ({ {
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_j=temp_j1;
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f();
}
}
}
}
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ {
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_link=(&&LINK1);
 ::TopLevel::f();
}
}
}
}
LINK1:;
}
 ::TopLevel::f_return; }); }); }); });
cease( (({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ {
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_j=temp_j2;
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_link=(&&LINK2);
 ::TopLevel::f();
}
}
}
}
LINK2:;
}
 ::TopLevel::f_return; }); }); }); })*(2))+ ::TopLevel::x );
return ;
}

void (TopLevel::f)()
{
auto short j;
auto int t;
auto char k;
/*temp*/ void *temp_link;
auto void *link;
auto int i;
link= ::TopLevel::f_link;
k= ::TopLevel::f_k;
j= ::TopLevel::f_j;
i= ::TopLevel::f_i;
t=(i+((3)*j));
{
 ::TopLevel::f_return=(t+(k*(5)));
{
temp_link=link;
return ;
}
}
}
