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
 ::TopLevel::x=(0);
{
/*temp*/ int result;
/*temp*/ int temp_i;
{
/*temp*/ int result1;
/*temp*/ int temp_i1;
temp_i1=(0);
{
/*temp*/ int result2;
/*temp*/ int temp_k;
temp_k=(0);
{
/*temp*/ int result3;
/*temp*/ int temp_j;
temp_j=(0);
{
/*temp*/ int result4;
{
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_link1=(&&LINK);
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
result4= ::TopLevel::f_return;
result3=result4;
}
result2=result3;
}
result1=result2;
}
temp_i=result1;
}
{
/*temp*/ int result5;
/*temp*/ int temp_j1;
temp_j1=(6);
{
/*temp*/ int result6;
/*temp*/ int temp_k1;
temp_k1=(8);
{
/*temp*/ int result7;
{
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_link1=(&&LINK1);
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_j=temp_j1;
goto ENTER_f;
}
}
}
}
LINK1:;
}
result7= ::TopLevel::f_return;
result6=result7;
}
result5=result6;
}
result=result5;
}
 ::TopLevel::x=result;
}
{
/*temp*/ int result8;
/*temp*/ int temp_i2;
temp_i2=(1);
{
/*temp*/ int result9;
/*temp*/ int temp_j2;
temp_j2=(2);
{
/*temp*/ int result10;
/*temp*/ int temp_k2;
temp_k2=(3);
{
/*temp*/ int result11;
{
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_link1=(&&LINK2);
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_j=temp_j2;
goto ENTER_f;
}
}
}
}
LINK2:;
}
result11= ::TopLevel::f_return;
result10=result11;
}
result9=result10;
}
result8=result9;
}
cease(  ::TopLevel::x+((2)*result8) );
}
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::link= ::TopLevel::f_link1;
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
