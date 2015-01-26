#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_i;
int x;
/*temp*/ void *f_link;
/*temp*/ short f_j;
/*temp*/ char f_k;
private:
int t;
void *link;
public:
/*temp*/ int f_return;
private:
int i;
short j;
public:
/*temp*/ void *f_link1;
void T();
private:
char k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_j;
/*temp*/ int temp_i;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_j1;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int temp_k;
/*temp*/ int result6;
/*temp*/ int temp_i1;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_j2;
/*temp*/ int result9;
/*temp*/ int temp_k1;
/*temp*/ int result10;
/*temp*/ int temp_i2;
/*temp*/ int temp_k2;
/*temp*/ void *temp_link;
/*temp*/ int result11;
auto void *state;
 ::TopLevel::x=(0);
temp_k=(8);
temp_j1=(6);
temp_j2=(0);
temp_i=(0);
temp_k1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link=(&&LINK);
wait(SC_ZERO_TIME);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result4= ::TopLevel::f_return;
result1=result4;
result9=result1;
result8=result9;
temp_i2=result8;
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link=(&&LINK1);
{
state=(&&ENTER_f);
goto *(state);
}
LINK1:;
result3= ::TopLevel::f_return;
result11=result3;
result2=result11;
result10=result2;
 ::TopLevel::x=result10;
temp_i1=(1);
temp_k2=(3);
temp_j=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link=(&&LINK2);
{
state=(&&ENTER_f);
goto *(state);
}
LINK2:;
result5= ::TopLevel::f_return;
result6=result5;
result=result6;
result7=result;
cease(  ::TopLevel::x+(result7*(2)) );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
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
