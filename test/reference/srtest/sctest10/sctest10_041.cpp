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
int i;
short j;
char k;
void *link;
int t;
public:
int x;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
/*temp*/ int result10;
/*temp*/ int result9;
/*temp*/ int result4;
/*temp*/ int result7;
/*temp*/ int result11;
/*temp*/ int result6;
/*temp*/ int result1;
/*temp*/ int result5;
/*temp*/ int result8;
/*temp*/ int result;
/*temp*/ int result2;
/*temp*/ int result3;
/*temp*/ int temp_i;
/*temp*/ int temp_i2;
/*temp*/ int temp_i1;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_j;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
/*temp*/ int temp_k;
 ::TopLevel::x=(0);
temp_i2=(0);
temp_k1=(0);
temp_j2=(0);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link1=(&&LINK);
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_j=temp_j2;
goto ENTER_f;
LINK:;
result1= ::TopLevel::f_return;
result11=result1;
result6=result11;
result7=result6;
temp_i=result7;
temp_j1=(6);
temp_k2=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link1=(&&LINK1);
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_j=temp_j1;
goto ENTER_f;
LINK1:;
result= ::TopLevel::f_return;
result8=result;
result5=result8;
result4=result5;
 ::TopLevel::x=result4;
temp_i1=(1);
temp_j=(2);
temp_k=(3);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link1=(&&LINK2);
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_j=temp_j;
goto ENTER_f;
LINK2:;
result3= ::TopLevel::f_return;
result9=result3;
result2=result9;
result10=result2;
cease(  ::TopLevel::x+((2)*result10) );
return ;
ENTER_f:;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+((3)* ::TopLevel::j));
 ::TopLevel::f_return=( ::TopLevel::t+((5)* ::TopLevel::k));
temp_link= ::TopLevel::link;
goto *(temp_link);
}
