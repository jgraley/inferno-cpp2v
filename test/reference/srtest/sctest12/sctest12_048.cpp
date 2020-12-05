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
void *link;
public:
int x;
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
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
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
break;
}
switch_value=(0);
wait(SC_ZERO_TIME);
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
goto CASE1;
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
goto THEN_ELSE_BREAK;
CASE2:;
goto *((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED3));
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
goto ELSE;
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
goto THEN_ELSE_BREAK;
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
goto ENTER_f;
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link1;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
goto *(temp_link);
}
