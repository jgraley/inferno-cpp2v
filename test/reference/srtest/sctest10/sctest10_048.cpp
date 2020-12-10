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
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int result10;
/*temp*/ int result11;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int result9;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_k;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
 ::TopLevel::x=(0);
temp_i=(0);
temp_k=(0);
temp_j=(0);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link=(&&LINK);
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
wait(SC_ZERO_TIME);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
result1=result;
result2=result1;
result3=result2;
temp_i1=result3;
temp_j1=(6);
temp_k1=(8);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link=(&&LINK1);
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k1;
goto ENTER_f;
LINK1:;
result4= ::TopLevel::f_return;
result5=result4;
result6=result5;
result7=result6;
 ::TopLevel::x=result7;
temp_i2=(1);
temp_j2=(2);
temp_k2=(3);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link=(&&LINK2);
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
goto ENTER_f;
LINK2:;
result8= ::TopLevel::f_return;
result9=result8;
result10=result9;
result11=result10;
cease(  ::TopLevel::x+((2)*result11) );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
