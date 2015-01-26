#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_k;
void T();
/*temp*/ int result2;
/*temp*/ int temp_j;
/*temp*/ int result3;
private:
int i;
public:
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_i;
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 1U,
};
/*temp*/ int temp_i1;
/*temp*/ int result7;
private:
char k;
public:
int x;
/*temp*/ int result8;
/*temp*/ int temp_j1;
/*temp*/ int f_i;
/*temp*/ int temp_j2;
private:
int t;
unsigned int state;
public:
/*temp*/ short f_j;
private:
unsigned int link;
public:
/*temp*/ int temp_i2;
private:
short j;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ char f_k;
/*temp*/ int result9;
/*temp*/ int temp_k1;
/*temp*/ int result10;
/*temp*/ int temp_k2;
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
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
 ::TopLevel::temp_k=(8);
 ::TopLevel::temp_j=(6);
 ::TopLevel::temp_j1=(0);
 ::TopLevel::temp_i=(0);
 ::TopLevel::temp_k2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result3= ::TopLevel::f_return;
 ::TopLevel::result2= ::TopLevel::result3;
 ::TopLevel::result1= ::TopLevel::result2;
 ::TopLevel::result4= ::TopLevel::result1;
 ::TopLevel::temp_i2= ::TopLevel::result4;
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result11= ::TopLevel::f_return;
 ::TopLevel::result8= ::TopLevel::result11;
 ::TopLevel::result5= ::TopLevel::result8;
 ::TopLevel::result9= ::TopLevel::result5;
 ::TopLevel::x= ::TopLevel::result9;
 ::TopLevel::temp_i1=(1);
 ::TopLevel::temp_k1=(3);
 ::TopLevel::temp_j2=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result10= ::TopLevel::f_return;
 ::TopLevel::result6= ::TopLevel::result10;
 ::TopLevel::result7= ::TopLevel::result6;
 ::TopLevel::result= ::TopLevel::result7;
cease(  ::TopLevel::x+( ::TopLevel::result*(2)) );
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
