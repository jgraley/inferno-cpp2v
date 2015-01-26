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
int t;
char k;
short j;
public:
/*temp*/ void *f_link;
int x;
/*temp*/ int f_i;
/*temp*/ char f_k;
private:
void *link;
public:
/*temp*/ short f_j;
void T();
/*temp*/ void *f_link1;
/*temp*/ int f_return;
private:
int i;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int result2;
/*temp*/ int temp_k;
/*temp*/ int result3;
/*temp*/ int temp_j;
/*temp*/ int result4;
/*temp*/ int temp_k1;
/*temp*/ int temp_j1;
/*temp*/ int result5;
/*temp*/ int temp_i2;
/*temp*/ int result6;
/*temp*/ int temp_k2;
/*temp*/ int result7;
/*temp*/ int result8;
/*temp*/ int temp_j2;
/*temp*/ int result9;
/*temp*/ void *temp_link;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k1=(0);
temp_i1=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
result6=result;
result11=result6;
result3=result11;
temp_i=result3;
temp_k2=(8);
temp_j1=(6);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result8= ::TopLevel::f_return;
result9=result8;
result4=result9;
result7=result4;
 ::TopLevel::x=result7;
temp_j=(2);
temp_k=(3);
temp_i2=(1);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result5= ::TopLevel::f_return;
result10=result5;
result2=result10;
result1=result2;
cease(  ::TopLevel::x+(result1*(2)) );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=(((3)* ::TopLevel::j)+ ::TopLevel::i);
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
