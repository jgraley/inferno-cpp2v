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
short j;
int t;
public:
int x;
private:
void *link;
public:
/*temp*/ short f_j;
/*temp*/ char f_k;
void T();
/*temp*/ void *f_link;
/*temp*/ int f_return;
private:
int i;
char k;
public:
/*temp*/ void *f_link1;
/*temp*/ int f_i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int temp_i;
/*temp*/ int temp_k;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int temp_i1;
/*temp*/ int temp_j;
/*temp*/ int temp_i2;
/*temp*/ int temp_j1;
/*temp*/ int result3;
/*temp*/ void *temp_link;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int result5;
/*temp*/ int temp_j2;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int temp_k2;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k=(0);
temp_i=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link=(&&LINK);
wait(SC_ZERO_TIME);
goto ENTER_f;
LINK:;
result11= ::TopLevel::f_return;
result10=result11;
result4=result10;
result6=result4;
temp_i2=result6;
temp_k1=(8);
temp_j=(6);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result3= ::TopLevel::f_return;
result5=result3;
result=result5;
result8=result;
 ::TopLevel::x=result8;
temp_j2=(2);
temp_k2=(3);
temp_i1=(1);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result1= ::TopLevel::f_return;
result7=result1;
result2=result7;
result9=result2;
cease( ((2)*result9)+ ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
