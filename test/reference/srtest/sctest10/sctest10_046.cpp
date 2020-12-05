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
/*temp*/ int result2;
/*temp*/ int result7;
/*temp*/ int result9;
/*temp*/ int result;
/*temp*/ int result3;
/*temp*/ int result8;
/*temp*/ int result10;
/*temp*/ int result6;
/*temp*/ int result5;
/*temp*/ int result4;
/*temp*/ int result1;
/*temp*/ int result11;
/*temp*/ int temp_i;
/*temp*/ int temp_i1;
/*temp*/ int temp_i2;
/*temp*/ int temp_j;
/*temp*/ int temp_j1;
/*temp*/ int temp_j2;
/*temp*/ int temp_k1;
/*temp*/ int temp_k2;
/*temp*/ int temp_k;
 ::TopLevel::x=(0);
temp_i1=(0);
temp_k1=(0);
temp_j1=(0);
 ::TopLevel::f_i=temp_i1;
 ::TopLevel::f_link1=(&&LINK);
 ::TopLevel::f_k=temp_k1;
 ::TopLevel::f_j=temp_j1;
goto ENTER_f;
LINK:;
result10= ::TopLevel::f_return;
result3=result10;
result8=result3;
result=result8;
temp_i=result;
temp_j=(6);
temp_k2=(8);
 ::TopLevel::f_i=temp_i;
 ::TopLevel::f_link1=(&&LINK1);
 ::TopLevel::f_k=temp_k2;
 ::TopLevel::f_j=temp_j;
goto ENTER_f;
LINK1:;
result4= ::TopLevel::f_return;
result5=result4;
result6=result5;
result9=result6;
 ::TopLevel::x=result9;
temp_i2=(1);
temp_j2=(2);
temp_k=(3);
 ::TopLevel::f_i=temp_i2;
 ::TopLevel::f_link1=(&&LINK2);
 ::TopLevel::f_k=temp_k;
 ::TopLevel::f_j=temp_j2;
goto ENTER_f;
LINK2:;
result11= ::TopLevel::f_return;
result7=result11;
result1=result7;
result2=result1;
cease(  ::TopLevel::x+((2)*result2) );
return ;
goto ENTER_f;
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
