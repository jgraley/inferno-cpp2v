#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ char f_k;
int x;
void T();
private:
unsigned int link;
char k;
int i;
public:
/*temp*/ int f_i;
private:
int t;
public:
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
};
private:
short j;
public:
/*temp*/ int f_return;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_k;
/*temp*/ int result3;
/*temp*/ int temp_j;
/*temp*/ int result4;
/*temp*/ int temp_i;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_k1;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int temp_j1;
auto unsigned int state;
/*temp*/ int temp_j2;
/*temp*/ int result11;
/*temp*/ int temp_i1;
/*temp*/ int temp_k2;
/*temp*/ int temp_i2;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_k=(0);
temp_i1=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result3= ::TopLevel::f_return;
result9=result3;
result10=result9;
result5=result10;
temp_i=result5;
temp_k1=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result2= ::TopLevel::f_return;
result7=result2;
result1=result7;
result=result1;
 ::TopLevel::x=result;
temp_j2=(2);
temp_k2=(3);
temp_i2=(1);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result11= ::TopLevel::f_return;
result6=result11;
result8=result6;
result4=result8;
cease(  ::TopLevel::x+((2)*result4) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
