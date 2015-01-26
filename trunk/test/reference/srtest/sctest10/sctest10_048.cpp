#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
/*temp*/ int f_i;
private:
char k;
int t;
public:
/*temp*/ void *f_link;
/*temp*/ short f_j;
/*temp*/ char f_k;
int x;
private:
void *link;
public:
/*temp*/ int f_return;
private:
int i;
short j;
public:
/*temp*/ void *f_link1;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int temp_k;
/*temp*/ int result1;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int temp_j;
/*temp*/ int temp_i;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ void *temp_link;
/*temp*/ int result9;
/*temp*/ int temp_i1;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
/*temp*/ int result10;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int result11;
/*temp*/ int temp_i2;
 ::TopLevel::x=(0);
temp_k2=(8);
temp_j2=(6);
temp_j=(0);
temp_i1=(0);
temp_k1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link=(&&LINK);
wait(SC_ZERO_TIME);
goto ENTER_f;
LINK:;
result2= ::TopLevel::f_return;
result11=result2;
result4=result11;
result1=result4;
temp_i2=result1;
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link=(&&LINK1);
goto ENTER_f;
LINK1:;
result8= ::TopLevel::f_return;
result9=result8;
result6=result9;
result=result6;
 ::TopLevel::x=result;
temp_i=(1);
temp_k=(3);
temp_j1=(2);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link=(&&LINK2);
goto ENTER_f;
LINK2:;
result7= ::TopLevel::f_return;
result3=result7;
result5=result3;
result10=result5;
cease( (result10*(2))+ ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
