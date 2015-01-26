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
/*temp*/ int f_return;
private:
int t;
public:
/*temp*/ unsigned int f_link;
private:
int i;
public:
int x;
/*temp*/ short f_j;
/*temp*/ char f_k;
private:
char k;
unsigned int link;
public:
void T();
/*temp*/ unsigned int f_link1;
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 1U,
};
private:
short j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_k;
static const unsigned int (lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_j;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_j1;
/*temp*/ int temp_k1;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int result8;
/*temp*/ int temp_k2;
/*temp*/ int result9;
/*temp*/ int result10;
auto unsigned int state;
/*temp*/ int temp_j2;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k2=(0);
temp_i=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK:;
result1= ::TopLevel::f_return;
result11=result1;
result=result11;
result2=result;
temp_i2=result2;
temp_k1=(8);
temp_j1=(6);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK1:;
result10= ::TopLevel::f_return;
result7=result10;
result9=result7;
result5=result9;
 ::TopLevel::x=result5;
temp_j=(2);
temp_k=(3);
temp_i1=(1);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
LINK2:;
result3= ::TopLevel::f_return;
result8=result3;
result4=result8;
result6=result4;
cease(  ::TopLevel::x+((2)*result6) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
goto *(lmap[state]);
}
