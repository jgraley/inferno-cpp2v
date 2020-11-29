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
private:
int i;
short j;
char k;
int t;
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int temp_i1;
/*temp*/ int temp_i;
/*temp*/ int temp_k1;
/*temp*/ int temp_j2;
/*temp*/ int temp_j1;
/*temp*/ int temp_k;
/*temp*/ int temp_i2;
/*temp*/ int temp_j;
/*temp*/ int temp_k2;
/*temp*/ int result8;
/*temp*/ int result4;
/*temp*/ int result;
/*temp*/ int result2;
/*temp*/ int result11;
/*temp*/ int result9;
/*temp*/ int result1;
/*temp*/ int result6;
/*temp*/ int result10;
/*temp*/ int result7;
/*temp*/ int result3;
/*temp*/ int result5;
 ::TopLevel::x=(0);
temp_i=(0);
temp_k1=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1=(&&LINK);
wait(SC_ZERO_TIME);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result2= ::TopLevel::f_return;
result11=result2;
result=result11;
result4=result;
temp_i1=result4;
temp_j1=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1=(&&LINK1);
{
state=(&&ENTER_f);
goto *(state);
}
LINK1:;
result9= ::TopLevel::f_return;
result6=result9;
result1=result6;
result8=result1;
 ::TopLevel::x=result8;
temp_i2=(1);
temp_j=(2);
temp_k2=(3);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1=(&&LINK2);
{
state=(&&ENTER_f);
goto *(state);
}
LINK2:;
result5= ::TopLevel::f_return;
result3=result5;
result7=result3;
result10=result7;
cease(  ::TopLevel::x+(result10*(2)) );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
