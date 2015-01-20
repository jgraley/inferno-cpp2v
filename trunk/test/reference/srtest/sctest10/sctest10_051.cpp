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
int i;
public:
enum TStates
{
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 2U,
T_STATE_ENTER_f = 3U,
T_STATE_LINK_2 = 0U,
};
/*temp*/ unsigned int f_link;
private:
char k;
public:
int x;
/*temp*/ short f_j;
private:
unsigned int link;
public:
/*temp*/ int f_i;
private:
short j;
public:
void T();
/*temp*/ char f_k;
/*temp*/ unsigned int f_link_1;
private:
int t;
public:
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int temp_i;
auto unsigned int state;
/*temp*/ unsigned int temp_link;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int temp_k;
/*temp*/ int result_3;
/*temp*/ int temp_j;
/*temp*/ int result_4;
/*temp*/ int temp_j_1;
/*temp*/ int temp_k_1;
/*temp*/ int temp_k_2;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int temp_i_1;
/*temp*/ int temp_i_2;
/*temp*/ int result_9;
/*temp*/ int temp_j_2;
/*temp*/ int result_10;
static const unsigned int (lmap[]) = { &&LINK, &&LINK_1, &&LINK_2, &&ENTER_f };
/*temp*/ int result_11;
 ::TopLevel::x=(0);
temp_i_1=(0);
temp_j=(0);
temp_k_2=(0);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_2;
wait(SC_ZERO_TIME);
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK:;
result_2= ::TopLevel::f_return;
result_10=result_2;
result_3=result_10;
result_7=result_3;
temp_i_2=result_7;
temp_j_2=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK;
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK_1:;
result_9= ::TopLevel::f_return;
result=result_9;
result_1=result;
result_11=result_1;
 ::TopLevel::x=result_11;
temp_k_1=(3);
temp_i=(1);
temp_j_1=(2);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link_1= ::TopLevel::T_STATE_LINK_1;
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_1;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
LINK_2:;
result_4= ::TopLevel::f_return;
result_5=result_4;
result_6=result_5;
result_8=result_6;
cease(  ::TopLevel::x+(result_8*(2)) );
return ;
{
state= ::TopLevel::T_STATE_ENTER_f;
goto *(lmap[state]);
}
ENTER_f:;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(lmap[state]);
}
}
