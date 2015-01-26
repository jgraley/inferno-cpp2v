#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
short j;
public:
/*temp*/ short f_j;
/*temp*/ char f_k;
private:
char k;
int t;
int i;
public:
/*temp*/ unsigned int f_link;
private:
unsigned int link;
public:
void T();
/*temp*/ int f_return;
/*temp*/ int f_i;
/*temp*/ unsigned int f_link1;
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
int x;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_k;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int result;
static const unsigned int (lmap[]) = { &&LINK, &&LINK1, &&LINK2, &&ENTER_f };
/*temp*/ unsigned int temp_link;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_k1;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_i1;
/*temp*/ int temp_k2;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
auto unsigned int state;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int temp_i2;
/*temp*/ int result11;
/*temp*/ int temp_j2;
 ::TopLevel::x=(0);
temp_k=(0);
temp_i2=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
LINK:;
if( state== ::TopLevel::T_STATE_LINK1 )
{
result9= ::TopLevel::f_return;
result2=result9;
result5=result2;
result3=result5;
temp_i1=result3;
temp_k1=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK1:;
if( state== ::TopLevel::T_STATE_LINK )
{
result6= ::TopLevel::f_return;
result1=result6;
result8=result1;
result7=result8;
 ::TopLevel::x=result7;
temp_j1=(2);
temp_k2=(3);
temp_i=(1);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
LINK2:;
if( state== ::TopLevel::T_STATE_LINK2 )
{
result= ::TopLevel::f_return;
result11=result;
result4=result11;
result10=result4;
cease(  ::TopLevel::x+((2)*result10) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
ENTER_f:;
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
