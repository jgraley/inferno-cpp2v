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
/*temp*/ int f_return;
/*temp*/ void *f_link;
private:
void *link;
public:
int x;
/*temp*/ void *f_link1;
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto int switch_value;
/*temp*/ int muxtemp;
/*temp*/ void *temp_link;
/*temp*/ int result;
/*temp*/ int result1;
auto int switch_value1;
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
switch_value=(0);
goto *((switch_value==(0)) ? (&&CASE1) : (&&PROCEED));
PROCEED:;
goto *((switch_value==(4)) ? (&&CASE) : (&&PROCEED1));
PROCEED1:;
goto *((switch_value==(1)) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
PROCEED_CASE:;
 ::TopLevel::x=(99);
goto BREAK;
CASE:;
 ::TopLevel::x=(44);
CASE1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
goto BREAK;
BREAK:;
switch_value1=(2);
goto *((switch_value1==(2)) ? (&&CASE2) : (&&PROCEED2));
PROCEED2:;
goto *((switch_value1==(1)) ? (&&PROCEED_CASE1) : (&&PROCEED_CASE1));
PROCEED_CASE1:;
 ::TopLevel::x=(99);
goto BREAK1;
CASE2:;
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE) : (&&PROCEED3));
PROCEED3:;
goto *((!(false)) ? (&&THEN) : (&&PROCEED4));
PROCEED4:;
muxtemp=(88);
goto ELSE;
THEN:;
 ::TopLevel::f_link1=(&&LINK);
goto ENTER_f;
LINK:;
result1= ::TopLevel::f_return;
muxtemp=result1;
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
goto THEN_ELSE;
THEN_ELSE:;
goto BREAK1;
BREAK1:;
cease(  ::TopLevel::x );
return ;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}