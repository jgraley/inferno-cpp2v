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
int x;
private:
void *link;
public:
/*temp*/ int f_return;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
/*temp*/ void *temp_link;
auto int switch_value;
auto int switch_value1;
/*temp*/ int muxtemp;
/*temp*/ int result;
/*temp*/ int result1;
 ::TopLevel::x=(0);
switch( 0 )
{
case 1:;
 ::TopLevel::x=(99);
break;
case 0:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch_value=(0);
goto *(((0)==switch_value) ? (&&CASE) : (&&PROCEED));
PROCEED:;
goto *(((4)==switch_value) ? (&&CASE1) : (&&PROCEED1));
PROCEED1:;
goto *(((1)==switch_value) ? (&&CASE2) : (&&PROCEED2));
PROCEED2:;
CASE2:;
 ::TopLevel::x=(99);
goto BREAK;
CASE1:;
 ::TopLevel::x=(44);
CASE:;
if( ((0)== ::TopLevel::x)||((2)== ::TopLevel::x) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK1;
BREAK1:;
BREAK:;
switch_value1=(2);
goto *(((2)==switch_value1) ? (&&CASE3) : (&&PROCEED3));
PROCEED3:;
goto *(((1)==switch_value1) ? (&&CASE4) : (&&PROCEED4));
PROCEED4:;
CASE4:;
 ::TopLevel::x=(99);
goto BREAK2;
CASE3:;
goto *((!(((0)== ::TopLevel::x)||((2)== ::TopLevel::x))) ? (&&THEN) : (&&PROCEED5));
PROCEED5:;
goto *((!(false)) ? (&&THEN1) : (&&PROCEED6));
PROCEED6:;
muxtemp=(88);
goto ELSE;
THEN1:;
 ::TopLevel::f_link=(&&LINK);
goto ENTER_f;
LINK:;
result= ::TopLevel::f_return;
muxtemp=result;
ELSE:;
result1=muxtemp;
 ::TopLevel::x=result1;
goto ELSE1;
THEN:;
ELSE1:;
goto BREAK3;
BREAK3:;
BREAK2:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
