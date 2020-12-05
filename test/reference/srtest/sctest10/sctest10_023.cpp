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
int i;
short j;
char k;
void *link;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
 ::TopLevel::x=({ /*temp*/ int temp_i; temp_i=({ /*temp*/ int temp_i1; temp_i1=(0); ({ /*temp*/ int temp_k; temp_k=(0); ({ /*temp*/ int temp_j; temp_j=(0); ({ {
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_link1=(&&LINK);
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_j=temp_j;
goto ENTER_f;
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
 ::TopLevel::f_link1=(&&LINK1);
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_j=temp_j1;
goto ENTER_f;
}
}
}
}
LINK1:;
}
 ::TopLevel::f_return; }); }); }); });
cease(  ::TopLevel::x+((2)*({ /*temp*/ int temp_i2; temp_i2=(1); ({ /*temp*/ int temp_j2; temp_j2=(2); ({ /*temp*/ int temp_k2; temp_k2=(3); ({ {
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_link1=(&&LINK2);
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_j=temp_j2;
goto ENTER_f;
}
}
}
}
LINK2:;
}
 ::TopLevel::f_return; }); }); }); })) );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
{
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
