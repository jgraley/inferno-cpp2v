#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
void T();
/*temp*/ int result;
private:
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int temp_k;
/*temp*/ int result_1;
private:
int i;
public:
/*temp*/ int temp_i;
int x;
/*temp*/ int temp_j;
/*temp*/ int result_2;
/*temp*/ int temp_k_1;
private:
char k;
public:
/*temp*/ int result_3;
/*temp*/ int temp_i_1;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int temp_i_2;
private:
short j;
public:
/*temp*/ int f_i;
/*temp*/ int result_6;
/*temp*/ int temp_j_1;
private:
int t;
public:
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_LINK_1 = 1U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK_2 = 0U,
};
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int temp_j_2;
/*temp*/ int temp_k_2;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_9;
/*temp*/ int result_10;
/*temp*/ int result_11;
private:
unsigned int link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i_2=(0);
 ::TopLevel::temp_j_2=(0);
 ::TopLevel::temp_k_1=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j= ::TopLevel::temp_j_2;
 ::TopLevel::f_k= ::TopLevel::temp_k_1;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
 ::TopLevel::result_6= ::TopLevel::f_return;
 ::TopLevel::result_11= ::TopLevel::result_6;
 ::TopLevel::result_2= ::TopLevel::result_11;
 ::TopLevel::result_4= ::TopLevel::result_2;
 ::TopLevel::temp_i_1= ::TopLevel::result_4;
 ::TopLevel::temp_j=(6);
 ::TopLevel::temp_k_2=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
 ::TopLevel::result_5= ::TopLevel::f_return;
 ::TopLevel::result_10= ::TopLevel::result_5;
 ::TopLevel::result_7= ::TopLevel::result_10;
 ::TopLevel::result_1= ::TopLevel::result_7;
 ::TopLevel::x= ::TopLevel::result_1;
 ::TopLevel::temp_k=(3);
 ::TopLevel::temp_i=(1);
 ::TopLevel::temp_j_1=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j= ::TopLevel::temp_j_1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result_8= ::TopLevel::f_return;
 ::TopLevel::result_9= ::TopLevel::result_8;
 ::TopLevel::result= ::TopLevel::result_9;
 ::TopLevel::result_3= ::TopLevel::result;
cease(  ::TopLevel::x+( ::TopLevel::result_3*(2)) );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
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
