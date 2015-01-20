#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
private:
unsigned int link;
public:
/*temp*/ short f_j;
private:
short j;
public:
/*temp*/ unsigned int f_link;
/*temp*/ char f_k;
private:
char k;
public:
/*temp*/ int f_return;
private:
int t;
public:
/*temp*/ unsigned int f_link_1;
/*temp*/ int f_i;
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK = 0U,
T_STATE_LINK_1 = 2U,
T_STATE_LINK_2 = 1U,
};
void T();
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_j;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int temp_i;
auto unsigned int state;
/*temp*/ unsigned int temp_link;
static const unsigned int (lmap[]) = { &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK, &&ENTER_f_LINK_LINK_LINK };
/*temp*/ int temp_k;
/*temp*/ int result_3;
/*temp*/ int temp_k_1;
/*temp*/ int result_4;
/*temp*/ int temp_j_1;
/*temp*/ int temp_k_2;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int temp_i_1;
/*temp*/ int result_7;
/*temp*/ int temp_j_2;
/*temp*/ int result_8;
/*temp*/ int temp_i_2;
/*temp*/ int result_9;
/*temp*/ int result_10;
/*temp*/ int result_11;
ENTER_f_LINK_LINK_LINK:;
if( (sc_delta_count())==(0U) )
{
 ::TopLevel::x=(0);
temp_i=(0);
temp_j_1=(0);
temp_k_2=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_2;
wait(SC_ZERO_TIME);
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
if(  ::TopLevel::T_STATE_LINK==state )
{
result_2= ::TopLevel::f_return;
result_6=result_2;
result_1=result_6;
result_10=result_1;
temp_i_1=result_10;
temp_j=(6);
temp_k_1=(8);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_1;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_2==state )
{
result_7= ::TopLevel::f_return;
result_9=result_7;
result=result_9;
result_8=result;
 ::TopLevel::x=result_8;
temp_k=(3);
temp_i_2=(1);
temp_j_2=(2);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_LINK_1==state )
{
result_5= ::TopLevel::f_return;
result_4=result_5;
result_3=result_4;
result_11=result_3;
cease( (result_11*(2))+ ::TopLevel::x );
return ;
state= ::TopLevel::T_STATE_ENTER_f;
}
if(  ::TopLevel::T_STATE_ENTER_f==state )
{
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
state=temp_link;
}
goto *(lmap[state]);
}
