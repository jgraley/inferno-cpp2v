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
 ::TopLevel::x=(0);
{
/*temp*/ int result;
/*temp*/ int temp_i;
{
/*temp*/ int result_1;
/*temp*/ int temp_i_1;
temp_i_1=(0);
{
/*temp*/ int result_2;
/*temp*/ int temp_k;
temp_k=(0);
{
/*temp*/ int result_3;
/*temp*/ int temp_j;
temp_j=(0);
{
/*temp*/ int result_4;
{
{
 ::TopLevel::f_i=temp_i_1;
{
 ::TopLevel::f_link=(&&LINK);
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_j=temp_j;
goto ENTER_f;
}
}
}
}
LINK:;
}
result_4= ::TopLevel::f_return;
result_3=result_4;
}
result_2=result_3;
}
result_1=result_2;
}
temp_i=result_1;
}
{
/*temp*/ int result_5;
/*temp*/ int temp_k_1;
temp_k_1=(8);
{
/*temp*/ int result_6;
/*temp*/ int temp_j_1;
temp_j_1=(6);
{
/*temp*/ int result_7;
{
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_link=(&&LINK_1);
{
 ::TopLevel::f_k=temp_k_1;
{
 ::TopLevel::f_j=temp_j_1;
goto ENTER_f;
}
}
}
}
LINK_1:;
}
result_7= ::TopLevel::f_return;
result_6=result_7;
}
result_5=result_6;
}
result=result_5;
}
 ::TopLevel::x=result;
}
{
/*temp*/ int result_8;
/*temp*/ int temp_j_2;
temp_j_2=(2);
{
/*temp*/ int result_9;
/*temp*/ int temp_k_2;
temp_k_2=(3);
{
/*temp*/ int result_10;
/*temp*/ int temp_i_2;
temp_i_2=(1);
{
/*temp*/ int result_11;
{
{
 ::TopLevel::f_i=temp_i_2;
{
 ::TopLevel::f_link=(&&LINK_2);
{
 ::TopLevel::f_k=temp_k_2;
{
 ::TopLevel::f_j=temp_j_2;
goto ENTER_f;
}
}
}
}
LINK_2:;
}
result_11= ::TopLevel::f_return;
result_10=result_11;
}
result_9=result_10;
}
result_8=result_9;
}
cease(  ::TopLevel::x+((2)*result_8) );
}
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
{
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
