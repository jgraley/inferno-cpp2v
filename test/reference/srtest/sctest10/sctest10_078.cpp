#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
int x;
/*temp*/ int f_return;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ unsigned int f_link;
private:
int t;
int i;
short j;
char k;
unsigned int link;
public:
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 1U,
T_STATE_LINK2 = 2U,
T_STATE_ENTER_f = 3U,
};
private:
unsigned int state;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ int result;
/*temp*/ int temp_k;
/*temp*/ int result1;
/*temp*/ int temp_j;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int result5;
/*temp*/ int temp_j1;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k2;
/*temp*/ int result8;
/*temp*/ int temp_j2;
/*temp*/ int result9;
/*temp*/ int temp_i1;
/*temp*/ int result10;
/*temp*/ int temp_i2;
void T();
/*temp*/ int result11;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i1=(0);
 ::TopLevel::temp_j2=(0);
 ::TopLevel::temp_k2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result7= ::TopLevel::f_return;
 ::TopLevel::result8= ::TopLevel::result7;
 ::TopLevel::result9= ::TopLevel::result8;
 ::TopLevel::result10= ::TopLevel::result9;
 ::TopLevel::temp_i2= ::TopLevel::result10;
 ::TopLevel::temp_j1=(6);
 ::TopLevel::temp_k1=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK1) )
{
 ::TopLevel::result4= ::TopLevel::f_return;
 ::TopLevel::result5= ::TopLevel::result4;
 ::TopLevel::result6= ::TopLevel::result5;
 ::TopLevel::result11= ::TopLevel::result6;
 ::TopLevel::x= ::TopLevel::result11;
 ::TopLevel::temp_i=(1);
 ::TopLevel::temp_j=(2);
 ::TopLevel::temp_k=(3);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK2) )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::result1= ::TopLevel::result;
 ::TopLevel::result2= ::TopLevel::result1;
 ::TopLevel::result3= ::TopLevel::result2;
cease(  ::TopLevel::x+( ::TopLevel::result3*(2)) );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_ENTER_f) )
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
if( enabled )
next_trigger(SC_ZERO_TIME);
}
