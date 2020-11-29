#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_LINK = 2U,
};
void T();
private:
int i;
short j;
char k;
unsigned int link;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ unsigned int temp_link;
auto unsigned int state;
/*temp*/ int result5;
/*temp*/ int result8;
/*temp*/ int result6;
/*temp*/ int result;
/*temp*/ int result10;
/*temp*/ int result1;
/*temp*/ int result11;
/*temp*/ int result9;
/*temp*/ int result7;
/*temp*/ int result2;
/*temp*/ int result4;
/*temp*/ int result3;
/*temp*/ int temp_i;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_k2;
/*temp*/ int temp_k1;
/*temp*/ int temp_k;
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i2=(0);
temp_k2=(0);
temp_j=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK1 )
{
result= ::TopLevel::f_return;
result10=result;
result8=result10;
result6=result8;
temp_i=result6;
temp_j1=(6);
temp_k1=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result7= ::TopLevel::f_return;
result9=result7;
result1=result9;
result5=result1;
 ::TopLevel::x=result5;
temp_i1=(1);
temp_j2=(2);
temp_k=(3);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result3= ::TopLevel::f_return;
result4=result3;
result2=result4;
result11=result2;
cease(  ::TopLevel::x+(result11*(2)) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
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
}
while( true );
}
