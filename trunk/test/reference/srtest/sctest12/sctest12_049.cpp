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
private:
void *link;
public:
/*temp*/ void *f_link;
/*temp*/ void *f_link1;
/*temp*/ int f_return;
void T();
};
TopLevel top_level("top_level");

void TopLevel::T()
{
auto int switch_value;
/*temp*/ int result;
/*temp*/ int result1;
/*temp*/ int muxtemp;
auto void *state;
/*temp*/ void *temp_link;
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
wait(SC_ZERO_TIME);
{
state=((switch_value==(0)) ? (&&CASE1) : (&&PROCEED));
goto *(state);
}
PROCEED:;
{
state=((switch_value==(4)) ? (&&CASE) : (&&PROCEED1));
goto *(state);
}
PROCEED1:;
{
state=((switch_value==(1)) ? (&&PROCEED_CASE) : (&&PROCEED_CASE));
goto *(state);
}
PROCEED_CASE:;
 ::TopLevel::x=(99);
{
state=(&&BREAK);
goto *(state);
}
CASE:;
 ::TopLevel::x=(44);
{
state=(&&CASE1);
goto *(state);
}
CASE1:;
if( ( ::TopLevel::x==(0))||( ::TopLevel::x==(2)) )
 ::TopLevel::x=((false) ? (88) : (2));
{
state=(&&BREAK);
goto *(state);
}
BREAK:;
switch_value1=(2);
{
state=((switch_value1==(2)) ? (&&CASE2) : (&&PROCEED2));
goto *(state);
}
PROCEED2:;
{
state=((switch_value1==(1)) ? (&&PROCEED_CASE1) : (&&PROCEED_CASE1));
goto *(state);
}
PROCEED_CASE1:;
 ::TopLevel::x=(99);
{
state=(&&THEN_ELSE_BREAK);
goto *(state);
}
CASE2:;
{
state=((!(( ::TopLevel::x==(0))||( ::TopLevel::x==(2)))) ? (&&THEN_ELSE_BREAK) : (&&PROCEED3));
goto *(state);
}
PROCEED3:;
{
state=((!(false)) ? (&&THEN) : (&&PROCEED4));
goto *(state);
}
PROCEED4:;
muxtemp=(88);
{
state=(&&ELSE);
goto *(state);
}
THEN:;
 ::TopLevel::f_link=(&&LINK);
{
state=(&&ENTER_f);
goto *(state);
}
LINK:;
result1= ::TopLevel::f_return;
muxtemp=result1;
{
state=(&&ELSE);
goto *(state);
}
ELSE:;
result=muxtemp;
 ::TopLevel::x=result;
{
state=(&&THEN_ELSE_BREAK);
goto *(state);
}
THEN_ELSE_BREAK:;
cease(  ::TopLevel::x );
return ;
{
state=(&&ENTER_f);
goto *(state);
}
ENTER_f:;
 ::TopLevel::link= ::TopLevel::f_link;
 ::TopLevel::f_return=(3);
temp_link= ::TopLevel::link;
{
state=temp_link;
goto *(state);
}
}
