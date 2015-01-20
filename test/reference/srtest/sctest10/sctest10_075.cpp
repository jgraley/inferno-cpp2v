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
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
private:
unsigned int link;
public:
/*temp*/ unsigned int temp_link;
private:
int i;
public:
/*temp*/ int result_4;
void T();
private:
unsigned int state;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int temp_k;
/*temp*/ int temp_i;
int x;
/*temp*/ int temp_j;
/*temp*/ int result_5;
/*temp*/ int temp_k_1;
private:
char k;
public:
/*temp*/ int result_6;
/*temp*/ int temp_i_1;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int temp_i_2;
private:
short j;
public:
/*temp*/ int f_i;
/*temp*/ int result_9;
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
/*temp*/ unsigned int f_link;
/*temp*/ int f_return;
/*temp*/ int result_10;
/*temp*/ int result_11;
/*temp*/ int temp_j_2;
/*temp*/ int temp_k_2;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
if( (0U)==(sc_delta_count()) )
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
return ;
}
if(  ::TopLevel::T_STATE_LINK_2== ::TopLevel::state )
{
 ::TopLevel::result_9= ::TopLevel::f_return;
 ::TopLevel::result_1= ::TopLevel::result_9;
 ::TopLevel::result_5= ::TopLevel::result_1;
 ::TopLevel::result_7= ::TopLevel::result_5;
 ::TopLevel::temp_i_1= ::TopLevel::result_7;
 ::TopLevel::temp_j=(6);
 ::TopLevel::temp_k_2=(8);
 ::TopLevel::f_i= ::TopLevel::temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j= ::TopLevel::temp_j;
 ::TopLevel::f_k= ::TopLevel::temp_k_2;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1== ::TopLevel::state )
{
 ::TopLevel::result_8= ::TopLevel::f_return;
 ::TopLevel::result= ::TopLevel::result_8;
 ::TopLevel::result_10= ::TopLevel::result;
 ::TopLevel::result_3= ::TopLevel::result_10;
 ::TopLevel::x= ::TopLevel::result_3;
 ::TopLevel::temp_k=(3);
 ::TopLevel::temp_i=(1);
 ::TopLevel::temp_j_1=(2);
 ::TopLevel::f_i= ::TopLevel::temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j= ::TopLevel::temp_j_1;
 ::TopLevel::f_k= ::TopLevel::temp_k;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK== ::TopLevel::state )
{
 ::TopLevel::result_11= ::TopLevel::f_return;
 ::TopLevel::result_2= ::TopLevel::result_11;
 ::TopLevel::result_4= ::TopLevel::result_2;
 ::TopLevel::result_6= ::TopLevel::result_4;
cease( ((2)* ::TopLevel::result_6)+ ::TopLevel::x );
return ;
 ::TopLevel::state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f== ::TopLevel::state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
 ::TopLevel::temp_link= ::TopLevel::link;
 ::TopLevel::state= ::TopLevel::temp_link;
}
next_trigger(SC_ZERO_TIME);
}
