#include "isystemc.h"

class TopLevel;
class TopLevel : public sc_module
{
public:
SC_CTOR( TopLevel )
{
SC_THREAD(T);
}
int x;
void T();
private:
int i;
short j;
char k;
int t;
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ int f_i;
/*temp*/ short f_j;
/*temp*/ char f_k;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
{
/*temp*/ int temp_i;
/*temp*/ int result;
{
/*temp*/ int temp_i1;
/*temp*/ int result1;
temp_i1=(0);
{
/*temp*/ int temp_j;
/*temp*/ int result2;
temp_j=(0);
{
/*temp*/ int temp_k;
/*temp*/ int result3;
temp_k=(0);
{
/*temp*/ int result4;
{
{
 ::TopLevel::f_i=temp_i1;
{
 ::TopLevel::f_j=temp_j;
{
 ::TopLevel::f_k=temp_k;
{
 ::TopLevel::f_link1=(&&LINK);
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
/*temp*/ int temp_j1;
/*temp*/ int result5;
temp_j1=(6);
{
/*temp*/ int temp_k1;
/*temp*/ int result6;
temp_k1=(8);
{
/*temp*/ int result7;
{
{
 ::TopLevel::f_i=temp_i;
{
 ::TopLevel::f_j=temp_j1;
{
 ::TopLevel::f_k=temp_k1;
{
 ::TopLevel::f_link1=(&&LINK1);
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
/*temp*/ int temp_i2;
/*temp*/ int result8;
temp_i2=(1);
{
/*temp*/ int temp_j2;
/*temp*/ int result9;
temp_j2=(2);
{
/*temp*/ int temp_k2;
/*temp*/ int result10;
temp_k2=(3);
{
/*temp*/ int result11;
{
{
 ::TopLevel::f_i=temp_i2;
{
 ::TopLevel::f_j=temp_j2;
{
 ::TopLevel::f_k=temp_k2;
{
 ::TopLevel::f_link1=(&&LINK2);
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
cease(  ::TopLevel::x+(result8*(2)) );
}
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::k= ::TopLevel::f_k;
 ::TopLevel::j= ::TopLevel::f_j;
 ::TopLevel::i= ::TopLevel::f_i;
 ::TopLevel::t=( ::TopLevel::i+( ::TopLevel::j*(3)));
{
 ::TopLevel::f_return=( ::TopLevel::t+( ::TopLevel::k*(5)));
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
