#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
private:
char k;
public:
/*temp*/ int f_return;
private:
int i;
unsigned int link;
public:
/*temp*/ short f_j;
private:
short j;
int t;
public:
/*temp*/ unsigned int f_link;
void T();
int x;
/*temp*/ int f_i;
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK = 0U,
T_STATE_LINK_1 = 2U,
T_STATE_LINK_2 = 1U,
};
/*temp*/ unsigned int f_link_1;
/*temp*/ char f_k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int temp_j;
/*temp*/ int temp_i;
/*temp*/ int result_1;
/*temp*/ int temp_j_1;
static const unsigned int (lmap[]) = { &&, &&, &&, && };
auto unsigned int state;
/*temp*/ int temp_k;
/*temp*/ int temp_i_1;
/*temp*/ int result_2;
/*temp*/ int temp_k_1;
/*temp*/ int result_3;
/*temp*/ int temp_k_2;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int result_7;
/*temp*/ int temp_i_2;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int result_10;
/*temp*/ int result_11;
/*temp*/ int temp_j_2;
/*temp*/ unsigned int temp_link;
do
{
if( (0U)==(sc_delta_count()) )
{
 ::TopLevel::x=(0);
temp_i=(0);
temp_j_1=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
continue;
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result= ::TopLevel::f_return;
result_5=result;
result_1=result_5;
result_8=result_1;
temp_i_2=result_8;
temp_j=(6);
temp_k_1=(8);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_2==state )
{
result_11= ::TopLevel::f_return;
result_3=result_11;
result_7=result_3;
result_10=result_7;
 ::TopLevel::x=result_10;
temp_k_2=(3);
temp_i_1=(1);
temp_j_2=(2);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k_2;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1==state )
{
result_4= ::TopLevel::f_return;
result_9=result_4;
result_6=result_9;
result_2=result_6;
cease(  ::TopLevel::x+(result_2*(2)) );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
state=temp_link;
}
}
while( true );
}
