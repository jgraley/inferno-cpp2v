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
void *link;
public:
/*temp*/ void *f_link;
int x;
void T();
/*temp*/ void *f_link1;
/*temp*/ int f_return;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
{
{
{
auto int switch_value;
switch_value=(0);
{
goto *(((0)==switch_value) ? (&&CASE2) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value) ? (&&CASE1) : (&&PROCEED1));
PROCEED1:;
goto *(((1)==switch_value) ? (&&CASE) : (&&PROCEED2));
PROCEED2:;
CASE:;
 ::TopLevel::x=(99);
goto BREAK1;
CASE1:;
 ::TopLevel::x=(44);
CASE2:;
if( ( ::TopLevel::x==(0))||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
}
}
BREAK:;
}
BREAK1:;
}
{
{
{
auto int switch_value1;
switch_value1=(2);
{
goto *(((2)==switch_value1) ? (&&CASE4) : (&&PROCEED3));
PROCEED3:;
goto *(((1)==switch_value1) ? (&&CASE3) : (&&PROCEED4));
PROCEED4:;
CASE3:;
 ::TopLevel::x=(99);
goto BREAK3;
CASE4:;
{
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN1) : (&&PROCEED5));
PROCEED5:;
{
/*temp*/ int muxtemp;
/*temp*/ int result;
{
goto *((!(false)) ? (&&THEN) : (&&PROCEED6));
PROCEED6:;
muxtemp=(88);
goto ELSE;
THEN:;
{
/*temp*/ int result1;
{
{
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
}
LINK:;
}
result1= ::TopLevel::f_return;
muxtemp=result1;
}
ELSE:;
}
result=muxtemp;
 ::TopLevel::x=result;
}
goto ELSE1;
THEN1:;
;
ELSE1:;
}
goto BREAK2;
}
}
BREAK2:;
}
BREAK3:;
}
cease(  ::TopLevel::x );
return ;
ENTER_f:;
{
/*temp*/ void *temp_link;
 ::TopLevel::link= ::TopLevel::f_link;
{
 ::TopLevel::f_return=(3);
{
temp_link= ::TopLevel::link;
goto *(temp_link);
}
}
}
}
