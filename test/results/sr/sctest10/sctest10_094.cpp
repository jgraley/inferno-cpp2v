#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_METHOD(T);
}
/*temp*/ unsigned int temp_link;
enum TStates
{
T_STATE_ENTER_f = 2,
T_STATE_LINK = 0,
T_STATE_LINK_1 = 1,
T_STATE_LINK_2 = 3,
};
void T();
private:
char k;
short j;
int i;
int t;
public:
int x;
private:
unsigned int link;
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_10;
/*temp*/ int result_11;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int temp_i;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
/*temp*/ int temp_j;
/*temp*/ int temp_j_1;
/*temp*/ int temp_j_2;
/*temp*/ int temp_k;
/*temp*/ int temp_k_1;
/*temp*/ int temp_k_2;
/*temp*/ unsigned int f_link;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ bool enabled = true;
if( (sc_delta_count())==(0) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i=(0);
 ::TopLevel::temp_j=(0);
 ::TopLevel::temp_k=(0);
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::f_i= ::TopLevel::temp_i;
next_trigger(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
enabled=(false);
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::result_1= ::TopLevel::result;
 ::TopLevel::result_2= ::TopLevel::result_1;
 ::TopLevel::result_3= ::TopLevel::result_2;
 ::TopLevel::temp_i_1= ::TopLevel::result_3;
 ::TopLevel::temp_k_1=(8);
 ::TopLevel::temp_j_1=(6);
 ::TopLevel::f_j= ::TopLevel::temp_j_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_k= ::TopLevel::temp_k_1;
 ::TopLevel::f_i= ::TopLevel::temp_i_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
 ::TopLevel::result_4= ::TopLevel::f_return;
 ::TopLevel::result_5= ::TopLevel::result_4;
 ::TopLevel::result_6= ::TopLevel::result_5;
 ::TopLevel::result_7= ::TopLevel::result_6;
 ::TopLevel::x= ::TopLevel::result_7;
 ::TopLevel::temp_j_2=(2);
 ::TopLevel::temp_k_2=(3);
 ::TopLevel::temp_i_2=(1);
 ::TopLevel::f_j= ::TopLevel::temp_j_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_k= ::TopLevel::temp_k_2;
 ::TopLevel::f_i= ::TopLevel::temp_i_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result_8= ::TopLevel::f_return;
 ::TopLevel::result_9= ::TopLevel::result_8;
 ::TopLevel::result_10= ::TopLevel::result_9;
 ::TopLevel::result_11= ::TopLevel::result_10;
cease(  ::TopLevel::x+((2)* ::TopLevel::result_11) );
enabled=(false);
if( enabled )
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
}
if( enabled )
{
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
}
if( enabled )
next_trigger(SC_ZERO_TIME);
}
