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
private:
char k;
short j;
int i;
int t;
public:
int x;
private:
void *link;
public:
void (f)();
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i_1; temp_i_1=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ {
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_link=(&&LINK);
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f();
}
}
}
}
LINK:;
}
 ::TopLevel::f_return; }); }); }); }); ({ /*temp*/ int temp_k_1; temp_k_1=(8); ({ /*temp*/ int temp_j_1; temp_j_1=(6); ({ {
{
 ::TopLevel::f_j=temp_j_1;
{
 ::TopLevel::f_link=(&&LINK_1);
{
 ::TopLevel::f_k=temp_k_1;
{
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f();
}
}
}
}
LINK_1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_j_2; temp_j_2=(2); ({ /*temp*/ int temp_k_2; temp_k_2=(3); ({ /*temp*/ int temp_i_2; temp_i_2=(1); ({ {
{
 ::TopLevel::f_j=temp_j_2;
{
 ::TopLevel::f_link=(&&LINK_2);
{
 ::TopLevel::f_k=temp_k_2;
{
 ::TopLevel::f_i=temp_i_2;
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
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
{
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
{
temp_link= ::TopLevel::link;
return ;
}
}
}
