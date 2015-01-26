#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_return;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link;
private:
unsigned int link;
short j;
public:
int x;
void T();
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 0U,
};
private:
char k;
int i;
int t;
public:
/*temp*/ char f_k;
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int temp_k;
/*temp*/ int result4;
/*temp*/ int temp_i1;
/*temp*/ int result5;
/*temp*/ int temp_j;
/*temp*/ int temp_k1;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_j1;
/*temp*/ int result8;
/*temp*/ int temp_k2;
/*temp*/ int result9;
/*temp*/ int temp_i2;
/*temp*/ int result10;
/*temp*/ unsigned int temp_link;
auto unsigned int state;
/*temp*/ int result11;
/*temp*/ int temp_j2;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_k1=(8);
temp_j2=(6);
temp_j=(0);
temp_i1=(0);
temp_k2=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if( state== ::TopLevel::T_STATE_LINK2 )
{
result4= ::TopLevel::f_return;
result7=result4;
result9=result7;
result11=result9;
temp_i=result11;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK1==state )
{
result5= ::TopLevel::f_return;
result=result5;
result8=result;
result3=result8;
 ::TopLevel::x=result3;
temp_i2=(1);
temp_k=(3);
temp_j1=(2);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
state= ::TopLevel::T_STATE_ENTER_f;
}
if( state== ::TopLevel::T_STATE_LINK )
{
result2= ::TopLevel::f_return;
result6=result2;
result1=result6;
result10=result1;
cease(  ::TopLevel::x+(result10*(2)) );
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
