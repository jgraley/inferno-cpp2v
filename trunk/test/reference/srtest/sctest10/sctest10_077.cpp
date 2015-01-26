#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ int result;
/*temp*/ int temp_j;
private:
unsigned int state;
int t;
public:
/*temp*/ int temp_j1;
void T();
/*temp*/ int result1;
/*temp*/ int temp_k;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ char f_k;
/*temp*/ int result4;
private:
unsigned int link;
public:
/*temp*/ int result5;
/*temp*/ int result6;
private:
short j;
public:
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int result7;
private:
int i;
public:
/*temp*/ int temp_k1;
int x;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ short f_j;
/*temp*/ unsigned int f_link;
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_LINK1 = 0U,
T_STATE_LINK2 = 1U,
T_STATE_ENTER_f = 3U,
};
/*temp*/ int result10;
/*temp*/ int temp_j2;
/*temp*/ int temp_k2;
/*temp*/ int f_i;
/*temp*/ int result11;
/*temp*/ unsigned int temp_link;
/*temp*/ int f_return;
private:
char k;
public:
/*temp*/ int temp_i2;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_k=(0);
 ::TopLevel::temp_i1=(0);
 ::TopLevel::temp_j2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK1) )
{
 ::TopLevel::result6= ::TopLevel::f_return;
 ::TopLevel::result5= ::TopLevel::result6;
 ::TopLevel::result9= ::TopLevel::result5;
 ::TopLevel::result4= ::TopLevel::result9;
 ::TopLevel::temp_i= ::TopLevel::result4;
 ::TopLevel::temp_k2=(8);
 ::TopLevel::temp_j1=(6);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK2) )
{
 ::TopLevel::result11= ::TopLevel::f_return;
 ::TopLevel::result7= ::TopLevel::result11;
 ::TopLevel::result3= ::TopLevel::result7;
 ::TopLevel::result8= ::TopLevel::result3;
 ::TopLevel::x= ::TopLevel::result8;
 ::TopLevel::temp_j=(2);
 ::TopLevel::temp_k1=(3);
 ::TopLevel::temp_i2=(1);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::state== ::TopLevel::T_STATE_LINK) )
{
 ::TopLevel::result10= ::TopLevel::f_return;
 ::TopLevel::result2= ::TopLevel::result10;
 ::TopLevel::result= ::TopLevel::result2;
 ::TopLevel::result1= ::TopLevel::result;
cease(  ::TopLevel::x+((2)* ::TopLevel::result1) );
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
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
