#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
private:
unsigned int state;
public:
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_i;
private:
int i;
public:
/*temp*/ unsigned int f_link;
/*temp*/ int result3;
/*temp*/ int temp_i1;
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 1U,
};
/*temp*/ int temp_i2;
/*temp*/ int result4;
private:
char k;
public:
/*temp*/ int result5;
/*temp*/ int temp_k;
/*temp*/ int result6;
/*temp*/ int result7;
private:
short j;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ char f_k;
/*temp*/ int result8;
/*temp*/ int temp_k1;
/*temp*/ int result9;
/*temp*/ int temp_k2;
/*temp*/ int f_return;
int x;
/*temp*/ int f_i;
/*temp*/ int temp_j;
/*temp*/ short f_j;
/*temp*/ int result10;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int result11;
private:
unsigned int link;
int t;
public:
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_k=(8);
 ::TopLevel::temp_j2=(6);
 ::TopLevel::temp_j1=(0);
 ::TopLevel::temp_i1=(0);
 ::TopLevel::temp_k2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result11= ::TopLevel::f_return;
 ::TopLevel::result7= ::TopLevel::result11;
 ::TopLevel::result6= ::TopLevel::result7;
 ::TopLevel::result= ::TopLevel::result6;
 ::TopLevel::temp_i= ::TopLevel::result;
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result3= ::TopLevel::f_return;
 ::TopLevel::result10= ::TopLevel::result3;
 ::TopLevel::result1= ::TopLevel::result10;
 ::TopLevel::result8= ::TopLevel::result1;
 ::TopLevel::x= ::TopLevel::result8;
 ::TopLevel::temp_i2=(1);
 ::TopLevel::temp_k1=(3);
 ::TopLevel::temp_j=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result9= ::TopLevel::f_return;
 ::TopLevel::result2= ::TopLevel::result9;
 ::TopLevel::result4= ::TopLevel::result2;
 ::TopLevel::result5= ::TopLevel::result4;
cease(  ::TopLevel::x+( ::TopLevel::result5*(2)) );
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
