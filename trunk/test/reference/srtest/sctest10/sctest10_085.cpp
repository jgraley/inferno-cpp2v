#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ short f_j;
void T();
/*temp*/ int f_return;
private:
unsigned int state;
short j;
char k;
public:
/*temp*/ int f_i;
private:
unsigned int link;
public:
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int temp_i;
/*temp*/ int temp_j;
/*temp*/ unsigned int f_link;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int result_7;
private:
int t;
public:
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int temp_j_1;
/*temp*/ char f_k;
/*temp*/ int temp_k;
/*temp*/ int temp_k_1;
/*temp*/ int result_10;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
enum TStates
{
T_STATE_LINK = 2U,
T_STATE_LINK_1 = 1U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK_2 = 0U,
};
int x;
/*temp*/ int result_11;
private:
int i;
public:
/*temp*/ unsigned int temp_link;
/*temp*/ int temp_j_2;
/*temp*/ int temp_k_2;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i=(0);
 ::TopLevel::temp_j=(0);
 ::TopLevel::temp_k_2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k_2;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled&&( ::TopLevel::T_STATE_LINK_2== ::TopLevel::state) )
{
 ::TopLevel::result_3= ::TopLevel::f_return;
 ::TopLevel::result_7= ::TopLevel::result_3;
 ::TopLevel::result_10= ::TopLevel::result_7;
 ::TopLevel::result_11= ::TopLevel::result_10;
 ::TopLevel::temp_i_2= ::TopLevel::result_11;
 ::TopLevel::temp_j_1=(6);
 ::TopLevel::temp_k=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j= ::TopLevel::temp_j_1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_LINK_1== ::TopLevel::state) )
{
 ::TopLevel::result_5= ::TopLevel::f_return;
 ::TopLevel::result_6= ::TopLevel::result_5;
 ::TopLevel::result_9= ::TopLevel::result_6;
 ::TopLevel::result_2= ::TopLevel::result_9;
 ::TopLevel::x= ::TopLevel::result_2;
 ::TopLevel::temp_k_1=(3);
 ::TopLevel::temp_i_1=(1);
 ::TopLevel::temp_j_2=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j= ::TopLevel::temp_j_2;
 ::TopLevel::f_k= ::TopLevel::temp_k_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_LINK== ::TopLevel::state) )
{
 ::TopLevel::result_8= ::TopLevel::f_return;
 ::TopLevel::result= ::TopLevel::result_8;
 ::TopLevel::result_1= ::TopLevel::result;
 ::TopLevel::result_4= ::TopLevel::result_1;
cease(  ::TopLevel::x+( ::TopLevel::result_4*(2)) );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if( enabled&&( ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state) )
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
if( enabled )
next_trigger(SC_ZERO_TIME);
}
