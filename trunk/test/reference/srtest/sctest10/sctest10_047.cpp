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
/*temp*/ char f_k;
/*temp*/ void *f_link;
/*temp*/ int f_i;
int x;
/*temp*/ void *f_link_1;
private:
char k;
public:
/*temp*/ int f_return;
private:
short j;
int t;
void *link;
int i;
public:
/*temp*/ short f_j;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_j;
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int temp_i;
/*temp*/ int result_2;
/*temp*/ int temp_i_1;
/*temp*/ int result_3;
/*temp*/ int result_4;
/*temp*/ int result_5;
/*temp*/ int result_6;
/*temp*/ int temp_k;
/*temp*/ int temp_k_1;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int result_9;
/*temp*/ int temp_k_2;
/*temp*/ int temp_i_2;
/*temp*/ int result_10;
/*temp*/ int result_11;
/*temp*/ void *temp_link;
/*temp*/ int temp_j_1;
/*temp*/ int temp_j_2;
 ::TopLevel::x=(0);
temp_i=(0);
temp_j_1=(0);
temp_k_1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link_1=(&&LINK);
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_1;
wait(SC_ZERO_TIME);
goto ENTER_f;
LINK:;
result_3= ::TopLevel::f_return;
result_5=result_3;
result_4=result_5;
result_8=result_4;
temp_i_1=result_8;
temp_j=(6);
temp_k_2=(8);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link_1=(&&LINK_1);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_2;
goto ENTER_f;
LINK_1:;
result_11= ::TopLevel::f_return;
result=result_11;
result_7=result;
result_9=result_7;
 ::TopLevel::x=result_9;
temp_k=(3);
temp_i_2=(1);
temp_j_2=(2);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link_1=(&&LINK_2);
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
goto ENTER_f;
LINK_2:;
result_10= ::TopLevel::f_return;
result_1=result_10;
result_2=result_1;
result_6=result_2;
cease( (result_6*(2))+ ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
