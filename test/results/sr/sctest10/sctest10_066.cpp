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
private:
char k;
short j;
int i;
int t;
public:
int x;
private:
void *link;
public:
/*temp*/ char f_k;
/*temp*/ short f_j;
/*temp*/ int f_i;
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link_1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
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
 ::TopLevel::x=(0);
temp_i=(0);
temp_j=(0);
temp_k=(0);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_i=temp_i;
wait(SC_ZERO_TIME);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
result_1=result;
result_2=result_1;
result_3=result_2;
temp_i_1=result_3;
temp_k_1=(8);
temp_j_1=(6);
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_link=(&&LINK_1);
 ::TopLevel::f_k=temp_k_1;
 ::TopLevel::f_i=temp_i_1;
goto ENTER_f;
LINK_1:;
result_4= ::TopLevel::f_return;
result_5=result_4;
result_6=result_5;
result_7=result_6;
 ::TopLevel::x=result_7;
temp_j_2=(2);
temp_k_2=(3);
temp_i_2=(1);
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_link=(&&LINK_2);
 ::TopLevel::f_k=temp_k_2;
 ::TopLevel::f_i=temp_i_2;
goto ENTER_f;
LINK_2:;
result_8= ::TopLevel::f_return;
result_9=result_8;
result_10=result_9;
result_11=result_10;
cease(  ::TopLevel::x+((2)*result_11) );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
