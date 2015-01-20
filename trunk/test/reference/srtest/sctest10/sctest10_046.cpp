#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_return;
void T();
/*temp*/ void *f_link;
private:
void *link;
public:
/*temp*/ short f_j;
/*temp*/ char f_k;
private:
int i;
short j;
int t;
public:
/*temp*/ int f_i;
int x;
/*temp*/ void *f_link_1;
private:
char k;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result_1;
/*temp*/ int result_2;
/*temp*/ int temp_j;
/*temp*/ int result_3;
/*temp*/ int temp_i;
/*temp*/ int result_4;
/*temp*/ int temp_k;
/*temp*/ int result_5;
/*temp*/ int temp_k_1;
/*temp*/ int result_6;
/*temp*/ int temp_i_1;
/*temp*/ int result_7;
/*temp*/ int result_8;
/*temp*/ int temp_j_1;
/*temp*/ int result_9;
/*temp*/ int temp_j_2;
/*temp*/ void *temp_link;
/*temp*/ int temp_k_2;
/*temp*/ int temp_i_2;
/*temp*/ int result_10;
/*temp*/ int result_11;
 ::TopLevel::x=(0);
temp_i_1=(0);
temp_j_2=(0);
temp_k=(0);
 ::TopLevel::f_i=temp_i_1;
 ::TopLevel::f_link_1=(&&LINK);
 ::TopLevel::f_j=temp_j_2;
 ::TopLevel::f_k=temp_k;
goto ENTER_f;
LINK:;
result_6= ::TopLevel::f_return;
result_2=result_6;
result_1=result_2;
result_8=result_1;
temp_i=result_8;
temp_j=(6);
temp_k_2=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link_1=(&&LINK_1);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k_2;
goto ENTER_f;
LINK_1:;
result_11= ::TopLevel::f_return;
result=result_11;
result_5=result;
result_10=result_5;
 ::TopLevel::x=result_10;
temp_k_1=(3);
temp_i_2=(1);
temp_j_1=(2);
 ::TopLevel::f_i=temp_i_2;
 ::TopLevel::f_link_1=(&&LINK_2);
 ::TopLevel::f_j=temp_j_1;
 ::TopLevel::f_k=temp_k_1;
goto ENTER_f;
LINK_2:;
result_3= ::TopLevel::f_return;
result_9=result_3;
result_4=result_9;
result_7=result_4;
cease(  ::TopLevel::x+(result_7*(2)) );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link_1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
