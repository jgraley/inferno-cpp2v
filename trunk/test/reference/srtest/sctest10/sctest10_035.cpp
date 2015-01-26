#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ char f_k;
/*temp*/ int f_return;
/*temp*/ int f_i;
int x;
private:
void *link;
char k;
public:
/*temp*/ void *f_link;
private:
short j;
public:
void T();
/*temp*/ short f_j;
private:
int i;
int t;
public:
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_k;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ void *temp_link;
/*temp*/ int result3;
/*temp*/ int temp_j;
/*temp*/ int result4;
/*temp*/ int temp_i1;
/*temp*/ int result5;
/*temp*/ int temp_j1;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int temp_k1;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int temp_k2;
/*temp*/ int temp_j2;
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_k1=(8);
temp_j2=(6);
temp_j1=(0);
temp_i2=(0);
temp_k2=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result11= ::TopLevel::f_return;
result9=result11;
result7=result9;
result8=result7;
temp_i=result8;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result10= ::TopLevel::f_return;
result6=result10;
result=result6;
result4=result;
 ::TopLevel::x=result4;
temp_i1=(1);
temp_k=(3);
temp_j=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result3= ::TopLevel::f_return;
result5=result3;
result2=result5;
result1=result2;
cease(  ::TopLevel::x+(result1*(2)) );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
