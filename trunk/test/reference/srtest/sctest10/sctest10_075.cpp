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
/*temp*/ int result;
/*temp*/ int temp_j;
/*temp*/ int f_i;
/*temp*/ int temp_j1;
private:
int t;
unsigned int state;
public:
/*temp*/ short f_j;
private:
unsigned int link;
public:
/*temp*/ int temp_j2;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_i;
private:
int i;
public:
/*temp*/ unsigned int f_link;
/*temp*/ int result5;
/*temp*/ int temp_i1;
enum TStates
{
T_STATE_LINK = 0U,
T_STATE_LINK1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK2 = 1U,
};
/*temp*/ int result6;
/*temp*/ int temp_i2;
private:
char k;
public:
/*temp*/ int result7;
/*temp*/ int temp_k;
/*temp*/ int result8;
/*temp*/ int result9;
void T();
private:
short j;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ char f_k;
/*temp*/ int result10;
/*temp*/ int temp_k1;
/*temp*/ int result11;
/*temp*/ int temp_k2;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_k=(8);
 ::TopLevel::temp_j2=(6);
 ::TopLevel::temp_j=(0);
 ::TopLevel::temp_i1=(0);
 ::TopLevel::temp_k2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i1;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
return ;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK )
{
 ::TopLevel::result1= ::TopLevel::f_return;
 ::TopLevel::result9= ::TopLevel::result1;
 ::TopLevel::result8= ::TopLevel::result9;
 ::TopLevel::result2= ::TopLevel::result8;
 ::TopLevel::temp_i= ::TopLevel::result2;
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_j= ::TopLevel::temp_j2;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK2== ::TopLevel::state )
{
 ::TopLevel::result5= ::TopLevel::f_return;
 ::TopLevel::result= ::TopLevel::result5;
 ::TopLevel::result3= ::TopLevel::result;
 ::TopLevel::result10= ::TopLevel::result3;
 ::TopLevel::x= ::TopLevel::result10;
 ::TopLevel::temp_i2=(1);
 ::TopLevel::temp_k1=(3);
 ::TopLevel::temp_j1=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i2;
 ::TopLevel::f_j= ::TopLevel::temp_j1;
 ::TopLevel::f_k= ::TopLevel::temp_k1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::state== ::TopLevel::T_STATE_LINK1 )
{
 ::TopLevel::result11= ::TopLevel::f_return;
 ::TopLevel::result4= ::TopLevel::result11;
 ::TopLevel::result6= ::TopLevel::result4;
 ::TopLevel::result7= ::TopLevel::result6;
cease(  ::TopLevel::x+( ::TopLevel::result7*(2)) );
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
next_trigger(SC_ZERO_TIME);
}
