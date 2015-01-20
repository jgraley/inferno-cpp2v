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
void T();
private:
short j;
public:
/*temp*/ int f_i;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
/*temp*/ int f_return;
private:
char k;
public:
/*temp*/ char f_k;
private:
int t;
void *link;
int i;
public:
/*temp*/ short f_j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_k;
/*temp*/ int result;
/*temp*/ int temp_j;
/*temp*/ int temp_i;
/*temp*/ int result_1;
/*temp*/ int temp_k_1;
/*temp*/ int result_2;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int temp_k_2;
/*temp*/ int temp_i_1;
/*temp*/ int temp_j_1;
/*temp*/ int temp_i_2;
/*temp*/ int result_5;
/*temp*/ int temp_j_2;
/*temp*/ void *temp_link;
auto void *state;
/*temp*/ int result_6;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int result_10;
/*temp*/ int result_11;
 ::TopLevel::x=(0);
temp_i_2=(0);
temp_j_2=(0);
temp_k_1=(0);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k_1;
wait(SC_ZERO_TIME);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result_6= ::TopLevel::f_return;
result_4=result_6;
result_9=result_4;
result_1=result_9;
temp_i=result_1;
temp_j_1=(6);
temp_k=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link=(&&LINK_1);
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k;
{
state=(&&ENTER_f);
goto *(state);
}
LINK_1:;
result_5= ::TopLevel::f_return;
result_2=result_5;
result_8=result_2;
result_10=result_8;
 ::TopLevel::x=result_10;
temp_k_2=(3);
temp_i_1=(1);
temp_j=(2);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link=(&&LINK_2);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_2;
{
state=(&&ENTER_f);
goto *(state);
}
LINK_2:;
result_11= ::TopLevel::f_return;
result=result_11;
result_7=result;
result_3=result_7;
cease(  ::TopLevel::x+(result_3*(2)) );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
