#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int temp_j;
/*temp*/ int result;
/*temp*/ int temp_i;
/*temp*/ int result1;
void T();
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int f_return;
/*temp*/ int temp_k;
/*temp*/ int result5;
/*temp*/ int temp_k1;
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
private:
int t;
short j;
public:
/*temp*/ char f_k;
private:
unsigned int link;
public:
/*temp*/ int result6;
private:
char k;
public:
/*temp*/ int result7;
int x;
/*temp*/ int temp_i1;
/*temp*/ int temp_j1;
private:
int i;
public:
/*temp*/ int temp_i2;
/*temp*/ int result8;
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
/*temp*/ int result9;
/*temp*/ int temp_j2;
/*temp*/ int temp_k2;
/*temp*/ int result10;
/*temp*/ int result11;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_k2=(0);
 ::TopLevel::temp_i2=(0);
 ::TopLevel::temp_j=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result7= ::TopLevel::f_return;
 ::TopLevel::result2= ::TopLevel::result7;
 ::TopLevel::result5= ::TopLevel::result2;
 ::TopLevel::result10= ::TopLevel::result5;
 ::TopLevel::temp_i1= ::TopLevel::result10;
 ::TopLevel::temp_k1=(8);
 ::TopLevel::temp_j2=(6);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result6= ::TopLevel::f_return;
 ::TopLevel::result4= ::TopLevel::result6;
 ::TopLevel::result= ::TopLevel::result4;
 ::TopLevel::result3= ::TopLevel::result;
 ::TopLevel::x= ::TopLevel::result3;
 ::TopLevel::temp_j1=(2);
 ::TopLevel::temp_k=(3);
 ::TopLevel::temp_i=(1);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::result11= ::TopLevel::result1;
 ::TopLevel::result9= ::TopLevel::result11;
 ::TopLevel::result8= ::TopLevel::result9;
cease(  ::TopLevel::x+( ::TopLevel::result8*(2)) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f )
{
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
