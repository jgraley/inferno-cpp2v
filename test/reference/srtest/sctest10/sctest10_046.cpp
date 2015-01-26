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
/*temp*/ char f_k;
/*temp*/ void *f_link;
private:
short j;
public:
void T();
/*temp*/ short f_j;
private:
int i;
char k;
int t;
public:
/*temp*/ void *f_link1;
int x;
private:
void *link;
public:
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ int result;
/*temp*/ void *temp_link;
/*temp*/ int result1;
/*temp*/ int temp_j;
/*temp*/ int result2;
/*temp*/ int temp_i;
/*temp*/ int result3;
/*temp*/ int result4;
/*temp*/ int result5;
/*temp*/ int result6;
/*temp*/ int temp_i1;
/*temp*/ int result7;
/*temp*/ int temp_k;
/*temp*/ int temp_j1;
/*temp*/ int temp_k1;
/*temp*/ int temp_i2;
/*temp*/ int result8;
/*temp*/ int temp_k2;
/*temp*/ int temp_j2;
/*temp*/ int result9;
/*temp*/ int result10;
/*temp*/ int result11;
 ::TopLevel::x=(0);
temp_k=(8);
temp_j1=(6);
temp_j=(0);
temp_i2=(0);
temp_k1=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_j=temp_j;
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result5= ::TopLevel::f_return;
result8=result5;
result2=result8;
result7=result2;
temp_i=result7;
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_j=temp_j1;
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_link1=(&&LINK1);
goto ENTER_f;
LINK1:;
result4= ::TopLevel::f_return;
result9=result4;
result11=result9;
result3=result11;
 ::TopLevel::x=result3;
temp_i1=(1);
temp_k2=(3);
temp_j2=(2);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_j=temp_j2;
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_link1=(&&LINK2);
goto ENTER_f;
LINK2:;
result6= ::TopLevel::f_return;
result1=result6;
result10=result1;
result=result10;
cease(  ::TopLevel::x+(result*(2)) );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=(( ::TopLevel::k*(5))+ ::TopLevel::t);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
