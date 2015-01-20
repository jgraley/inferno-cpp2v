#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
private:
short j;
public:
int x;
private:
int i;
int t;
public:
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 2U,
T_STATE_LINK_2 = 0U,
T_STATE_ENTER_f = 3U,
};
private:
char k;
unsigned int link;
public:
void T();
private:
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_j;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int temp_i;
/*temp*/ int temp_k;
/*temp*/ int temp_i_1;
/*temp*/ int temp_j_1;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int result_7;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int temp_k_1;
/*temp*/ int temp_k_2;
/*temp*/ int temp_i_2;
/*temp*/ int temp_j_2;
/*temp*/ int result_10;
/*temp*/ int result_11;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_i_2=(0);
temp_j_1=(0);
temp_k_2=(0);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_2;
wait(SC_ZERO_TIME);
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
result_9= ::TopLevel::f_return;
result_3=result_9;
result_7=result_3;
result_11=result_7;
temp_i=result_11;
temp_j=(6);
temp_k_1=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_1;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
result_5= ::TopLevel::f_return;
result_10=result_5;
result_6=result_10;
result_4=result_6;
 ::TopLevel::x=result_4;
temp_k=(3);
temp_i_1=(1);
temp_j_2=(2);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
result_1= ::TopLevel::f_return;
result_8=result_1;
result=result_8;
result_2=result;
cease(  ::TopLevel::x+(result_2*(2)) );
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
temp_link= ::TopLevel::link;
 ::TopLevel::state=temp_link;
}
wait(SC_ZERO_TIME);
}
while( true );
}
