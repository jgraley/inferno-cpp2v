#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
void T();
/*temp*/ void *f_link;
/*temp*/ char f_k;
private:
int t;
public:
/*temp*/ int f_i;
enum TStates
{
T_STATE_ENTER_f = 3U,
T_STATE_LINK = 1U,
T_STATE_LINK_1 = 0U,
T_STATE_LINK_2 = 2U,
};
/*temp*/ int f_return;
int x;
/*temp*/ void *f_link_1;
private:
int i;
void *link;
public:
/*temp*/ short f_j;
private:
short j;
char k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int temp_i;
/*temp*/ int result_6;
/*temp*/ void *temp_link;
/*temp*/ int temp_k;
/*temp*/ int temp_j;
/*temp*/ int result_7;
/*temp*/ int temp_k_1;
/*temp*/ int temp_k_2;
/*temp*/ int temp_i_1;
/*temp*/ int temp_j_1;
/*temp*/ int result_8;
static const void *(lmap[]) = { &&LINK, &&LINK_1, &&LINK_2, &&ENTER_f };
/*temp*/ int temp_i_2;
/*temp*/ int result_9;
/*temp*/ int result_10;
auto void *state;
/*temp*/ int result_11;
/*temp*/ int temp_j_2;
 ::TopLevel::x=(0);
temp_i=(0);
temp_j_1=(0);
temp_k_1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link_1=(lmap[ ::TopLevel::T_STATE_LINK_1]);
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_1;
wait(SC_ZERO_TIME);
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK:;
result_11= ::TopLevel::f_return;
result_4=result_11;
result_1=result_4;
result_9=result_1;
temp_i_2=result_9;
temp_j_2=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link_1=(lmap[ ::TopLevel::T_STATE_LINK]);
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK_1:;
result_8= ::TopLevel::f_return;
result_7=result_8;
result_10=result_7;
result_6=result_10;
 ::TopLevel::x=result_6;
temp_k_2=(3);
temp_i_1=(1);
temp_j=(2);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link_1=(lmap[ ::TopLevel::T_STATE_LINK_2]);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_2;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
}
LINK_2:;
result= ::TopLevel::f_return;
result_3=result;
result_2=result_3;
result_5=result_2;
cease(  ::TopLevel::x+(result_5*(2)) );
return ;
{
state=(lmap[ ::TopLevel::T_STATE_ENTER_f]);
goto *(state);
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
goto *(state);
}
}
