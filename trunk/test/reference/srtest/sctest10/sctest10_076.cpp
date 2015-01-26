#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ int temp_i;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_j;
void T();
/*temp*/ int result2;
/*temp*/ int result3;
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
/*temp*/ int temp_k;
/*temp*/ int result4;
/*temp*/ int f_return;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k1;
/*temp*/ int temp_j1;
/*temp*/ int temp_i1;
private:
short j;
public:
/*temp*/ char f_k;
private:
unsigned int link;
public:
/*temp*/ int result8;
private:
char k;
public:
/*temp*/ int result9;
int x;
/*temp*/ int temp_i2;
/*temp*/ int temp_j2;
private:
int i;
public:
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int temp_k2;
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link;
/*temp*/ int f_i;
private:
int t;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_k=(0);
 ::TopLevel::temp_i=(0);
 ::TopLevel::temp_j1=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result9= ::TopLevel::f_return;
 ::TopLevel::result11= ::TopLevel::result9;
 ::TopLevel::result7= ::TopLevel::result11;
 ::TopLevel::result4= ::TopLevel::result7;
 ::TopLevel::temp_i2= ::TopLevel::result4;
 ::TopLevel::temp_k1=(8);
 ::TopLevel::temp_j=(6);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result8= ::TopLevel::f_return;
 ::TopLevel::result10= ::TopLevel::result8;
 ::TopLevel::result3= ::TopLevel::result10;
 ::TopLevel::result6= ::TopLevel::result3;
 ::TopLevel::x= ::TopLevel::result6;
 ::TopLevel::temp_j2=(2);
 ::TopLevel::temp_k2=(3);
 ::TopLevel::temp_i1=(1);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK2 )
{
 ::TopLevel::result5= ::TopLevel::f_return;
 ::TopLevel::result2= ::TopLevel::result5;
 ::TopLevel::result1= ::TopLevel::result2;
 ::TopLevel::result= ::TopLevel::result1;
cease(  ::TopLevel::x+((2)* ::TopLevel::result) );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
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
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
