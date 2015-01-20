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
/*temp*/ int temp_j;
/*temp*/ int temp_k;
/*temp*/ int f_i;
private:
short j;
public:
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int temp_j_1;
private:
int t;
public:
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK_1 = 0U,
T_STATE_LINK_2 = 2U,
};
/*temp*/ int f_return;
/*temp*/ unsigned int f_link;
private:
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int temp_k_1;
/*temp*/ int temp_i;
int x;
/*temp*/ int temp_j_2;
/*temp*/ int result_3;
/*temp*/ int temp_k_2;
private:
char k;
public:
/*temp*/ int result_4;
/*temp*/ int temp_i_1;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int temp_i_2;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int result_10;
private:
unsigned int link;
public:
/*temp*/ unsigned int temp_link;
private:
int i;
public:
/*temp*/ int result_11;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
do
{
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
 ::TopLevel::temp_i_2=(0);
 ::TopLevel::temp_j=(0);
 ::TopLevel::temp_k_2=(0);
 ::TopLevel::f_i= ::TopLevel::temp_i_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k_2;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
 ::TopLevel::result_2= ::TopLevel::f_return;
 ::TopLevel::result_8= ::TopLevel::result_2;
 ::TopLevel::result_3= ::TopLevel::result_8;
 ::TopLevel::result_5= ::TopLevel::result_3;
 ::TopLevel::temp_i_1= ::TopLevel::result_5;
 ::TopLevel::temp_j_2=(6);
 ::TopLevel::temp_k=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j= ::TopLevel::temp_j_2;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result_6= ::TopLevel::f_return;
 ::TopLevel::result_7= ::TopLevel::result_6;
 ::TopLevel::result_1= ::TopLevel::result_7;
 ::TopLevel::result_10= ::TopLevel::result_1;
 ::TopLevel::x= ::TopLevel::result_10;
 ::TopLevel::temp_k_1=(3);
 ::TopLevel::temp_i=(1);
 ::TopLevel::temp_j_1=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j= ::TopLevel::temp_j_1;
 ::TopLevel::f_k= ::TopLevel::temp_k_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
 ::TopLevel::result= ::TopLevel::f_return;
 ::TopLevel::result_9= ::TopLevel::result;
 ::TopLevel::result_11= ::TopLevel::result_9;
 ::TopLevel::result_4= ::TopLevel::result_11;
cease(  ::TopLevel::x+( ::TopLevel::result_4*(2)) );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
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
wait(SC_ZERO_TIME);
}
while( true );
}
